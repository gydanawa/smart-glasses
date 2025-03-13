from google import genai
from google.genai import types

import PIL.Image
import os
import time

client = genai.Client(api_key="AIzaSyCRGXhdUjLRwPTfMWXdZVgaectucXey2JM")

num_cars_a = 0
num_cars_b = 0
num_cars_c = 0
total_pics = 0

directory = "PhotosOfOncomingCar"
start_time = time.perf_counter()
for filename in os.listdir(directory):
        
    image = PIL.Image.open(os.path.join(directory, filename))
    print(f"Processing {filename}...")

    response = client.models.generate_content(
        model="gemini-2.0-flash-lite-preview-02-05",
        contents=["Describe this scene in one sentence.", image])

    answer = response.text.lower()

    total_pics = total_pics + 1

    if "road" in answer or "street" in answer:
        num_cars_a = num_cars_a + 1

    response = client.models.generate_content(
        model="gemini-2.0-flash-lite-preview-02-05",
        contents=["This camera is mounted on glasses used by a visually impaired person. Give a one sentence description that will help them navigate the world.", image])
    
    answer = response.text.lower()
    
    print(answer)

    if "road" in answer or "street" in answer:
        num_cars_b = num_cars_b + 1

    response = client.models.generate_content(
        model="gemini-2.0-flash-lite-preview-02-05",
        contents=["Describe this picture focusing mainly on hazards for a pedestrian.", image])
    
    answer = response.text.lower()

    if "road" in answer or "street" in answer:
        num_cars_c = num_cars_c + 1

print("\nTEST CONCLUDED:  \"road\"")

print("Prompt: Describe this scene in one sentence.")
print(f"\tPercent of responses containing \"road\": {num_cars_a/total_pics*100}%")
print("Prompt: This camera is mounted on glasses used by a visually impaired person. Give a one sentence description that will help them navigate the world.")
print(f"\tPercent of responses containing \"road\": {num_cars_b/total_pics*100}%")
print("Prompt: Describe this picture focusing mainly on hazards for a pedestrian.")
print(f"\tPercent of responses containing \"road\": {num_cars_c/total_pics*100}%")
