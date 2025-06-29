// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>		// chdir
#include <sys/time.h>	// gettimeofday
#include <sys/param.h>	// MAXPATHLEN
#include <pigpio.h>

#ifdef _DEBUG
#define PROGRAM_DEBUG " (debug)"
#else
#define PROGRAM_DEBUG ""
#endif // _DEBUG

#define PROGRAM_VERSION					"0.1a" PROGRAM_DEBUG

#define PROGRAM_NAME					"presepiopi v" PROGRAM_VERSION
#define PROGRAM_AUTHOR					"by A. Ribeca <a.ribeca@gmail.com>"
#define PROGRAM_BUILD					"compiled on "__DATE__" "__TIME__

#define INPUT_FILENAME					"presepiopi.txt"
#define AUDIO_FILENAME					"audio.mp3"

#ifdef _DEBUG
#define PRINT_DEBUG(...) printf_dt("[DEBUG] "); printf(__VA_ARGS__)
#else
#define PRINT_DEBUG(...)
#endif // _DEBUG

static const int LED_PIN = 24;

static const int INVALID_VALUE = -9999;

#define BUTTON_IMPLEMENTATION
#include "button.h"

// use local time
static int printf_dt(const char* msg, ...)
{
#define BUF_SIZE 255

	char buf[BUF_SIZE+1] = { 0 };
	char dt[96] = { 0 };
	int ret;
	time_t t;
	struct tm tm;
	va_list args;
	
	static FILE* f;

	va_start(args, msg);
	vsnprintf(buf, BUF_SIZE, msg, args);
	va_end(args);
	
	t = time(NULL);
	tm = *localtime(&t); // for UTC use gmtime
	
	if ( ! f )
	{
		snprintf(dt, sizeof(dt), "%04d%02d%02d.log"	, tm.tm_year + 1900
													, tm.tm_mon + 1
													, tm.tm_mday
		);
	
		f = fopen(dt, "a");
	}
		
	snprintf(dt, sizeof(dt), "%04d/%02d/%02d %02d:%02d:%02d", tm.tm_year + 1900
															, tm.tm_mon + 1
															, tm.tm_mday
															, tm.tm_hour
															, tm.tm_min
															, tm.tm_sec
	);

	if ( f )	
	{
		fprintf(f, "[%s] %s", dt, buf);	
		fflush(f);
	}
	ret = fprintf(stdout, "[%s] %s", dt, buf);	
	fflush(stdout);
		
	return ret;
	
#undef BUF_SIZE
}

static void Led_Init(void)
{
	gpioSetMode(LED_PIN, PI_OUTPUT);
}

static void Led_On(void)
{
	gpioWrite(LED_PIN, PI_HIGH);
}

static void Led_Off(void)
{
	gpioWrite(LED_PIN, PI_LOW);
}

static void Led_Flash(float speed, int time)
{
	int i;
	
	for ( i = 0; i < time; ++i)
	{
		Led_On();
		gpioSleep(PI_TIME_RELATIVE, 0, speed * 1000000); 
		Led_Off();
		gpioSleep(PI_TIME_RELATIVE, 0, speed * 1000000); 
	}

	// turn off
	gpioWrite(LED_PIN, PI_LOW);
}

#define RELAYS_COUNT 8
static const int RELAYS[RELAYS_COUNT] = { 5, 6, 13, 16, 19, 20, 21, 26 };

typedef struct
{
	int secs;
	int relays[RELAYS_COUNT];
	
} ROW;

static void Relays_Init(void)
{
	int i;
	
	for ( i = 0; i < RELAYS_COUNT; ++i )
	{
		gpioSetMode(RELAYS[i], PI_OUTPUT);
	}
}

static void Relays_SetState(int relayIndex, int state)
{
    gpioWrite(RELAYS[relayIndex], ! state);
}

static void Relays_Off(void)
{
	int i;
	
	for ( i = 0; i < RELAYS_COUNT; ++i )
	{
		Relays_SetState(i, PI_LOW);
	}
}

static ROW* ParseInputFile(char* filename, int* rowsCount) 
{
	FILE* f;
	ROW* rows = NULL;
	
	*rowsCount = 0;
	
	f = fopen(filename, "r");
	if ( ! f )
		printf_dt("unable to open %s\n", filename);
	else
	{
	#define BUF_SIZE 255
	
		int err = 0;
	
		char buf[BUF_SIZE+1] = { 0 };
		
		while ( fgets(buf, BUF_SIZE, f) )
		{
			int secs;
			int relays[RELAYS_COUNT];
			int r = sscanf(buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d"	, &secs
																, relays
																, relays + 1
																, relays + 2
																, relays + 3
																, relays + 4
																, relays + 5
																, relays + 6
																, relays + 7
			);
			
			if ( r != 9 )
			{
				printf_dt("row %d has no 9 values!\n", *rowsCount + 1);
				err = 1;
				break;
			}
			else
			{
				int i;
				for ( i = 0; i < RELAYS_COUNT; ++i )
				{
					if ( (relays[i] < 0) || (relays[i] > 1) )
					{
						printf_dt("value %d at row %d must be between 0 and 1!\n", i + 1, *rowsCount + 1);
						err = 1;
						break;
					}
				}
				
				if ( err )
					break;
				else
				{
					ROW* rows_no_leak = realloc(rows, (*rowsCount+1)*sizeof(ROW));
					if ( ! rows_no_leak )
					{
						printf_dt("out of memory at row %d\n", *rowsCount + 1);
						err = 1;
						break;
					}
					else
					{
						rows = rows_no_leak;
						rows[*rowsCount].secs = secs;
						memcpy(rows[*rowsCount].relays, relays, RELAYS_COUNT*sizeof*rows[*rowsCount].relays);
						
						++(*rowsCount);
					}
				}
			}
		}
		
		fclose(f);
		
		if ( err )
		{
			free(rows);
			rows = NULL;
		}
	}
	
	return rows;
}

static int Path_Set(const char* path)
{
	return ! (-1 == chdir(path));
}

static const char* Path_GetProgramPath(void)
{
	static char buf[MAXPATHLEN] = { 0 };
	const char* p = NULL;

	if ( readlink( "/proc/self/exe", buf, MAXPATHLEN) != -1 )
	{
		buf[(strrchr(buf, '/')-buf)+1] = '\0';
		p = buf;
	}

    return p;
}

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_ALSA
#define MA_NO_ENCODING
#define MA_NO_WAV
#define MA_NO_FLAC
#define MA_NO_RESOURCE_MANAGER
#define MA_NO_NODE_GRAPH
#define MA_NO_ENGINE
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static int Audio_IsPlaying;

static void Audio_Callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount)
{
    ma_decoder* decoder = (ma_decoder*)device->pUserData;
    if ( decoder )
	{
		if ( Audio_IsPlaying )
			ma_decoder_read_pcm_frames(decoder, output, frameCount, NULL);
		else
			memset(output, 0, frameCount * ma_get_bytes_per_frame(device->playback.format, device->playback.channels));
    }

	// prevent warning
    (void)input;
}

static int Audio_Init(ma_decoder* decoder, ma_device* device, const char* filename)
{
	int ret = 0; // defaults to err
	
    if (MA_SUCCESS == ma_decoder_init_file(filename, NULL, decoder) )
	{
		ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
		deviceConfig.playback.format   = decoder->outputFormat;
		deviceConfig.playback.channels = decoder->outputChannels;
		deviceConfig.sampleRate        = decoder->outputSampleRate;
		deviceConfig.dataCallback      = Audio_Callback;
		deviceConfig.pUserData         = decoder;

		if ( ma_device_init(NULL, &deviceConfig, device) != MA_SUCCESS )
			ma_decoder_uninit(decoder);
		else
			ret = 1;
	}

    return ret;
}

static void Audio_Deinit(ma_decoder* decoder, ma_device* device)
{
    ma_device_uninit(device);
    ma_decoder_uninit(decoder);
}

int main(void)
{
	int ret = 1; // defaults to err
	
	printf_dt("\n");
	printf_dt(PROGRAM_NAME "\n");
	printf_dt(PROGRAM_AUTHOR "\n");
	printf_dt(PROGRAM_BUILD "\n");
	printf_dt("\n");
	
	const char* programPath = Path_GetProgramPath();
	if ( ! programPath )
		printf_dt("unable to get program path!\n");
	else if ( ! Path_Set(programPath) )
		printf_dt("unable to set current path!\n");
	else if ( gpioInitialise() < 0 )
		printf_dt("unable to init pigpio!\n");
	else
	{
		if ( ! Button_Init() )
			printf_dt("unable to init button!\n");
		else
		{
			Led_Init();
			
			Relays_Init();
			
			Relays_Off();
			
			{
				int rowsCount;
				ROW* rows = ParseInputFile(INPUT_FILENAME, &rowsCount);
				if ( rows )
				{				
				#if _DEBUG
					int i;
					
					for ( i = 0; i < rowsCount; ++i )
					{
						printf_dt("%d,%d,%d,%d,%d,%d,%d,%d,%d\n", rows[i].secs
																, rows[i].relays[0]
																, rows[i].relays[1]
																, rows[i].relays[2]
																, rows[i].relays[3]
																, rows[i].relays[4]
																, rows[i].relays[5]
																, rows[i].relays[6]
																, rows[i].relays[7]
						);
					}
				#endif // _DEBUG
				
					// audio file exist ?
					if ( -1 == access(AUDIO_FILENAME, F_OK) )
						printf_dt(AUDIO_FILENAME " do not exists!\n");
					else
					{
						ma_decoder decoder;
						ma_device device;
						
						if ( ! Audio_Init(&decoder, &device, AUDIO_FILENAME) )
							printf_dt("unable to init audio device!\n");
						else
						{
							char oldBuf[256] = { 0 };
							uint32_t lastTick = 0;
							int start = 0;
							int onPlay = 0;
							int currentRow = 0;
							double remain = 0;
							
							uint32_t lastBlinkTick = 0;
							const float blinkInterval = 0.5f;
							int ledStateDuringPause = 0;
							
							// start paused ;)
							ma_device_start(&device);
							
							Led_On();
							
							
							while ( 1 )
							{
								float bp = Button_GetPressDuration();
								
								// blink on pause
								if ( start && ! Audio_IsPlaying )
								{
									uint32_t currentTick = gpioTick();
									float elapsed = (currentTick - lastBlinkTick) / 1e6;
									
									if ( elapsed >= blinkInterval )
									{
										ledStateDuringPause = ! ledStateDuringPause;
										gpioWrite(LED_PIN, ledStateDuringPause);
										lastBlinkTick = currentTick;
									}
								}
								
								if ( bp )
								{							
									if ( bp > 1.5 )
									{
										PRINT_DEBUG("bp > 1.5 - reset\n");
										
										start = 0;
										onPlay = 0;
										Audio_IsPlaying = 0;
										
										Led_Flash(0.1, 3);
										Led_On();
										
										Relays_Off();
									}
									else
									{
										PRINT_DEBUG("bp < 1.5 - pause\n");
										
										if ( ! start )
										{
											PRINT_DEBUG("start\n");
									
											onPlay = 1;									
											currentRow = 0;
											remain = INVALID_VALUE;
											
											Led_Off();
											
											Relays_Off();
											
											// rewind
											ma_decoder_seek_to_pcm_frame(&decoder, 0);
											Audio_IsPlaying = 1;
											lastTick = gpioTick();
											start = 1;
										}
										else
										{										
											Audio_IsPlaying = ! Audio_IsPlaying;
											if ( Audio_IsPlaying )
											{
												Led_Off();
												lastTick = gpioTick();												
											}
											else
											{
												ledStateDuringPause = 1;
												Led_On();
												lastBlinkTick = gpioTick();
											}
											
											PRINT_DEBUG("Audio_IsPlaying = %d\n", Audio_IsPlaying);
										}
									}
								}
															
								if ( onPlay )
								{								
									if ( INVALID_VALUE == remain )
									{
										Led_Off();
										remain = rows[currentRow].secs;
										
										PRINT_DEBUG("remain set to %.2f\n", remain);
									}
									else if ( remain <= 0.f )
									{								
										int i;

										for ( i = 0; i < RELAYS_COUNT; ++i )
											Relays_SetState(i, rows[currentRow].relays[i]);
										++currentRow;
										if ( currentRow >= rowsCount )
										{
											onPlay = 0;
											start = 0;
											Led_On();
											printf_dt("done!                                                       \r");
										}
										else
											remain = INVALID_VALUE;
									}
									else
									{
										char buf[256] = { 0 };
										if ( Audio_IsPlaying )
										{
											sprintf(buf, "waiting for %dsecs...%.1f          ", rows[currentRow].secs, remain);
											if ( strcmp(buf, oldBuf) )
											{
												printf_dt(buf);
												printf("\r");
												fflush(stdout);
												strcpy(oldBuf, buf);
											}

											uint32_t currentTick = gpioTick();
											double elapsed = ((double)(currentTick - lastTick)) / 1e6;
											remain -= elapsed;
											lastTick = currentTick;
										}
									}
								}
								
								gpioDelay(fmin(remain * 1e6, 100000));
							}
							
							Audio_Deinit(&decoder, &device);
							
							ret = 0;
						}
					}
					
					free(rows);
				}
			}
		}
		
		gpioTerminate();
	}
	
    return ret;
}
