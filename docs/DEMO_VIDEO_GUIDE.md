# Demo Video Recording Guide

## What to Include in Your Demo Video

The demo video should be **1-2 minutes** and demonstrate:
1. Running the panorama stitching system
2. Showing input images
3. Displaying the stitching process
4. Presenting final panoramas
5. Brief look at experimental results

## Quick Demo Script

```bash
# 1. Show the project structure
ls -la

# 2. Display input images
ls datasets/*/

# 3. Run a quick panorama stitch (indoor scene)
./scripts/run_panorama.sh --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --detector orb --output demo.jpg

# 4. Show the result
# Open demo.jpg in an image viewer

# 5. Run with AKAZE for comparison
./scripts/run_panorama.sh --stitch datasets/indoor_scene/img1.jpg datasets/indoor_scene/img2.jpg --detector akaze --output demo_akaze.jpg

# 6. Show multi-image stitching
./scripts/run_panorama.sh --stitch datasets/outdoor_scene1/img1.jpg datasets/outdoor_scene1/img2.jpg datasets/outdoor_scene1/img3.jpg --output multi_demo.jpg

# 7. Show the analysis report
# Open results_analysis/analysis_report.html in browser
```

## Recording Tools

### Linux:
```bash
# Using SimpleScreenRecorder
sudo apt-get install simplescreenrecorder

# Using OBS Studio
sudo apt-get install obs-studio

# Command line recording with ffmpeg
ffmpeg -video_size 1920x1080 -framerate 30 -f x11grab -i :0.0 -c:v libx264 -qp 0 -preset ultrafast demo_video.mp4
```

### macOS:
- Use QuickTime Player (File > New Screen Recording)
- Or OBS Studio: https://obsproject.com/

### Windows:
- Use Windows Game Bar (Win + G)
- Or OBS Studio

## Video Script Narration

"Hello, I'll demonstrate my panorama stitching system for the Visual Computing assignment.

First, let me show the input images - we have indoor and outdoor scenes with multiple overlapping images.

Now I'll run the stitching with ORB detector... [run command]
The system detects features, matches them, and estimates the homography.

Here's the result - a seamless panorama created from two images.

Let's try AKAZE for comparison... [run command]
AKAZE provides more robust features but takes slightly longer.

Finally, here's multi-image stitching combining three images... [run command]

The system successfully handles multiple images with proper alignment and blending.

For detailed analysis, I've run 48 experiments comparing different configurations, all documented in the PDF report."

## Tips for Good Demo Video

1. **Keep it concise** - Focus on key functionality
2. **Show visual results** - Display panoramas clearly
3. **Explain briefly** - What's happening at each step
4. **Good quality** - 1080p if possible
5. **Clear narration** - Explain what you're demonstrating

## Compression (if needed)

```bash
# Compress video while maintaining quality
ffmpeg -i demo_video.mp4 -c:v libx264 -crf 23 -preset medium -c:a aac -b:a 128k demo_video_compressed.mp4
```