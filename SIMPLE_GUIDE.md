# 📸 Panorama Stitching - Simple Guide

## What This Project Does

Takes two overlapping photos and combines them into one wide panorama image.

## How It Works (Simple Version)

1. **Find Interesting Points** - Detects corners, edges, unique patterns in both images
2. **Match Points** - Finds which points in image 1 correspond to points in image 2  
3. **Remove Bad Matches** - Uses RANSAC to filter out incorrect matches
4. **Align Images** - Calculates how to transform one image to align with the other
5. **Blend Together** - Merges the images smoothly to create the panorama

## The Two Detectors Explained

### ORB (Fast but Less Accurate)
- Finds LOTS of points (25,000-50,000)
- Very fast processing
- Good for real-time applications
- May have more errors

### AKAZE (Slower but More Accurate)  
- Finds fewer but better points (5,000-20,000)
- Takes more time to process
- Better matching accuracy
- More reliable results

## Understanding the Results

### In the Visualizations:
- **Green Circles** = Detected keypoints (interesting points)
- **Lines Between Images** = Matched points
- **Thick Lines** = Good matches after filtering
- **Thin Lines** = Potential matches (may be wrong)

### In the Panoramas:
- **Good Result** = Smooth blending, no visible seams
- **OK Result** = Some artifacts but images are aligned
- **Failed Result** = Black areas or misaligned images

## Quick Test

```bash
# Test with your own images
./scripts/run_panorama.sh --stitch photo1.jpg photo2.jpg --output my_panorama.jpg
```

## Tips for Good Results

1. Images should overlap by 30-40%
2. Take photos from same position (just rotate camera)
3. Avoid moving objects in the overlap area
4. Good lighting helps detection

## Common Issues

- **Black areas**: Not enough good matches found
- **Misalignment**: Images taken from different positions
- **Blurry seams**: Motion blur or focus differences