#!/usr/bin/env python3
"""
Adapter to convert C++ experiment CSV format to match Python analysis expectations.
"""
import pandas as pd
import sys
import os

def convert_csv(input_file='results/metrics.csv', output_file='results/metrics_fixed.csv'):
    """Convert C++ CSV format to match analyze_results.py expectations"""
    
    if not os.path.exists(input_file):
        print(f"Error: {input_file} not found")
        return False
    
    # Read the C++ generated CSV
    df = pd.read_csv(input_file)
    print(f"Loaded {len(df)} experiments from C++ output")
    
    # Check if it's already in the right format
    if 'scene' in df.columns and 'status' in df.columns:
        print("CSV already in correct format")
        return True
    
    # Add missing columns that Python expects
    
    # Extract scene from experiment name (e.g., "indoor_scene_orb_pair1-2" -> "indoor_scene")
    if 'experiment' in df.columns:
        df['scene'] = df['experiment'].str.extract(r'(indoor_scene|outdoor_scene\d*)')
    else:
        df['scene'] = 'unknown_scene'
    
    # Add status based on whether inliers > 20 (typical success criteria)
    if 'num_inliers' in df.columns:
        df['status'] = df['num_inliers'].apply(lambda x: 'SUCCESS' if x > 20 else 'FAILED')
    else:
        df['status'] = 'SUCCESS'  # Assume success if we got results
    
    # Rename columns to match expected format
    rename_map = {
        'ransac_threshold': 'threshold',
        'num_keypoints_1': 'keypoints1',
        'num_keypoints_2': 'keypoints2',
        'num_matches': 'matches',
        'num_inliers': 'inliers'
    }
    
    for old_col, new_col in rename_map.items():
        if old_col in df.columns and new_col not in df.columns:
            df[new_col] = df[old_col]
    
    # Add images column (not critical but expected)
    df['images'] = 'img1.jpg-img2.jpg'
    
    # Save the fixed CSV
    df.to_csv(output_file, index=False)
    print(f"Fixed CSV saved to {output_file}")
    
    # Also overwrite the original for seamless integration
    df.to_csv(input_file, index=False)
    print(f"Original CSV updated at {input_file}")
    
    return True

if __name__ == "__main__":
    if len(sys.argv) > 1:
        convert_csv(sys.argv[1])
    else:
        convert_csv()
