from google import genai
from google.genai import types

import PIL.Image
import os
import time

client = genai.Client(api_key="AIzaSyCNlcIOl0tEP3oSzyzULcodc07hK0EiuF8")

num_cars = 0
num_crosswalks = 0
num_roads = 0
total_pics = 0

directory = "PhotosOfIntersectionStreet"
start_time = time.perf_counter()
for filename in os.listdir(directory):
        
    image = PIL.Image.open(os.path.join(directory, filename))
    print(f"Processing {filename}...")

    response = client.models.generate_content(
        model="gemini-2.0-flash-lite-preview-02-05",
        contents=["Describe this scene in one sentence.", image])

    answer = response.text.lower()

    print(answer)

    total_pics = total_pics + 1

    if "car " in answer or "cars " in answer or "vehicles" in answer:
        num_cars = num_cars + 1

    if "crosswalk" in answer or "intersection" in answer:
        num_crosswalks = num_crosswalks + 1

    if "road" in answer or "street" in answer:
        num_roads = num_roads + 1

print("\nTEST CONCLUDED:  Intersection with no cars")

print(f"Percent of pictures labelled with 'car': {num_cars/total_pics*100}%")
print(f"Percent of pictures labelled with 'crosswalk': {num_crosswalks/total_pics*100}%")
print(f"Percent of pictures labelled with 'road': {num_roads/total_pics*100}%")



num_cars = 0
num_crosswalks = 0
num_roads = 0
total_pics = 0

directory = "PhotosOfOncomingCar"
for filename in os.listdir(directory):
        
    image = PIL.Image.open(os.path.join(directory, filename))
    print(f"Processing {filename}...")

    response = client.models.generate_content(
        model="gemini-2.0-flash-lite-preview-02-05",
        contents=["This camera is mounted on glasses used by a visually impaired person. Give a one sentence description that will help them navigate life.", image])

    answer = response.text.lower()

    print(answer)

    total_pics = total_pics + 1

    if "car " in answer or "cars " in answer or "vehicles" in answer:
        num_cars = num_cars + 1

    if "crosswalk" in answer or "intersection" in answer:
        num_crosswalks = num_crosswalks + 1

    if "road" in answer or "street" in answer:
        num_roads = num_roads + 1


print("\nTEST CONCLUDED:  Oncoming car")

print(f"Percent of pictures labelled with 'car': {num_cars/total_pics*100}%")
print(f"Percent of pictures labelled with 'crosswalk': {num_crosswalks/total_pics*100}%")
print(f"Percent of pictures labelled with 'road': {num_roads/total_pics*100}%")



num_cars = 0
num_crosswalks = 0
num_roads = 0
total_pics = 0

directory = "PhotosOfSidewalk"
for filename in os.listdir(directory):
        
    image = PIL.Image.open(os.path.join(directory, filename))
    print(f"Processing {filename}...")

    response = client.models.generate_content(
        model="gemini-2.0-flash-lite-preview-02-05",
        contents=["This camera is mounted on glasses used by a visually impaired person. Give a one sentence description that will help them navigate life.", image])

    answer = response.text.lower()

    print(answer)

    total_pics = total_pics + 1

    if "car " in answer or "cars " in answer or "vehicles" in answer:
        num_cars = num_cars + 1

    if "crosswalk" in answer or "sidewalk" in answer or "path" in answer:
        num_crosswalks = num_crosswalks + 1

    if "road" in answer or "street" in answer or "intersection" in answer:
        num_roads = num_roads + 1


print("\nTEST CONCLUDED:  Sidewalk")

print(f"Percent of pictures labelled with 'car': {num_cars/total_pics*100}%")
print(f"Percent of pictures labelled with 'crosswalk': {num_crosswalks/total_pics*100}%")
print(f"Percent of pictures labelled with 'road': {num_roads/total_pics*100}%")

end_time = time.perf_counter()

print(f"Average image description time: {(end_time-start_time)/20:.2f}s")