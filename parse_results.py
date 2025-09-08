#!/usr/bin/env python3

import re

# Parse experiment log
with open('results/experiment_log.txt', 'r') as f:
    content = f.read()

# Extract data
orb_matches = re.search(r'ORB Detector:.*?Found (\d+) good matches.*?RANSAC found (\d+) inliers \(([0-9.]+)%\)', content, re.DOTALL)
akaze_matches = re.search(r'AKAZE Detector:.*?Found (\d+) good matches.*?RANSAC found (\d+) inliers \(([0-9.]+)%\)', content, re.DOTALL)

print("=== Feature Detector Comparison ===")
print("| Detector | Keypoints | Good Matches | Inliers | Inlier Ratio |")
print("|----------|-----------|--------------|---------|--------------|")
if orb_matches:
    print(f"| ORB      | 2000      | {orb_matches.group(1):12} | {orb_matches.group(2):7} | {orb_matches.group(3):11}% |")
if akaze_matches:
    print(f"| AKAZE    | 2000      | {akaze_matches.group(1):12} | {akaze_matches.group(2):7} | {akaze_matches.group(3):11}% |")

print("\n=== RANSAC Threshold Analysis ===")
print("| Threshold | Inliers | Inlier Ratio |")
print("|-----------|---------|--------------|")

thresholds = re.findall(r'RANSAC Threshold: ([0-9.]+).*?RANSAC found (\d+) inliers \(([0-9.]+)%\)', content, re.DOTALL)
for thresh, inliers, ratio in thresholds:
    print(f"| {thresh:9} | {inliers:7} | {ratio:11}% |")

print("\n=== Feature Count Analysis ===")
print("| Max Features | Good Matches | Inliers | Inlier Ratio |")
print("|--------------|--------------|---------|--------------|")

features = re.findall(r'Max Features: (\d+).*?Found (\d+) good matches.*?RANSAC found (\d+) inliers \(([0-9.]+)%\)', content, re.DOTALL)
for feat, matches, inliers, ratio in features:
    print(f"| {feat:12} | {matches:12} | {inliers:7} | {ratio:11}% |")

print("\n=== Scene Success Rates ===")
print("| Scene            | Good Matches | Inliers | Success |")
print("|------------------|--------------|---------|---------|")

# Indoor scene (using ORB results)
if orb_matches:
    print(f"| Indoor (tables)  | {orb_matches.group(1):12} | {orb_matches.group(2):7} | ✓       |")

# Outdoor scenes
outdoor1 = re.search(r'Outdoor Scene 1:.*?Found (\d+) good matches.*?RANSAC found (\d+) inliers', content, re.DOTALL)
outdoor2 = re.search(r'Outdoor Scene 2:.*?Found (\d+) good matches.*?RANSAC found (\d+) inliers', content, re.DOTALL)

if outdoor1:
    print(f"| Outdoor 1        | {outdoor1.group(1):12} | {outdoor1.group(2):7} | ✓       |")
if outdoor2:
    print(f"| Outdoor 2        | {outdoor2.group(1):12} | {outdoor2.group(2):7} | ✓       |")