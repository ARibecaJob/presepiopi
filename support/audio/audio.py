from pydub import AudioSegment
from pydub.generators import Sine
import os
import sys
import csv
import random

filepath = "../../bin/presepiopi.txt"

def create_beeps(sequence, output_filename="audio.mp3"):
    audio = AudioSegment.silent(duration=0)
    
    frequencies = [random.randint(220, 440) for _ in range(10)]
    
    for idx, num_beeps in enumerate(sequence):
        freq = frequencies[idx % len(frequencies)]
        
        for _ in range(num_beeps):
            beep = Sine(freq).to_audio_segment(duration=200)
            
            audio += beep
            audio += AudioSegment.silent(duration=800)

    path = os.path.join("../../bin", output_filename)      
    audio.export(path, format="mp3")
    print(f"File MP3 created in '{path}'")

def generate_prelesepio(sequence):
    with open(filepath, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        
        for i, value in enumerate(sequence):
            cycle_pos = i % 9
            if cycle_pos == 8:
                row = [value] + [0] * 8
            else:
                row = [value] + [1 if j == cycle_pos else 0 for j in range(8)]
            print(row)
            writer.writerow(row)

if __name__ == "__main__":
    input_from_args = False
    sequence = []
    
    if len(sys.argv) > 1:
        input_from_args = True
        try:
            sequence = list(map(int, sys.argv[1:]))
            
        except ValueError:
            print("args must be integer number separated by space")
            sequence = []
    else:
        if not os.path.exists(filepath):
            print("txt file not found in bin!")
        else:
            with open(filepath, newline='') as csvfile:
                reader = csv.reader(csvfile)
                for line_num, row in enumerate(reader, start=1):
                    if not row or len(row) < 9:
                        print(f"[row {line_num}] invalid format: {row}")
                        sequence = []
                        break

                    try:
                        duration = int(row[0])
                        flags = list(map(int, row[1:9]))
                    except ValueError:
                        print(f"[row {line_num}] not a number: {row}")
                        sequence = []
                        break

                    if not all(flag in (0, 1) for flag in flags):
                        print(f"[row {line_num}] number must be 0 or 1: {row}")
                        sequence = []
                        break

                    sequence.append(duration)
                print("imported sequence: ", sequence)

    if sequence:
        if input_from_args:
            generate_prelesepio(sequence)
        create_beeps(sequence)
