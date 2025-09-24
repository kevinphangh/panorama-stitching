#!/usr/bin/env python3
"""
Analysis pipeline for panorama stitching experiment results.
Processes CSV metrics and organizes results into folders.

Usage:
    python3 scripts/analysis_pipeline.py
"""

import os
import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
import shutil
import glob

# Set style for better looking plots
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'ggplot')
plt.rcParams['figure.figsize'] = (10, 6)
plt.rcParams['font.size'] = 10

def fix_csv_format(input_file='results/metrics.csv', output_file=None):
    """Convert C++ experiment CSV format to match Python analysis expectations"""

    if not os.path.exists(input_file):
        print(f"Error: {input_file} not found")
        return False

    try:
        df = pd.read_csv(input_file)
        print(f"Loaded {len(df)} experiments from C++ output")
    except Exception as e:
        print(f"Error reading CSV: {e}")
        return False

    if output_file:
        df.to_csv(output_file, index=False)
        print(f"Saved formatted CSV to {output_file}")

    return True

def load_experiment_data(csv_path='results/metrics.csv'):
    """Load and preprocess experiment data from CSV."""

    if not os.path.exists(csv_path):
        print(f"Error: {csv_path} not found")
        return None

    df = pd.read_csv(csv_path)

    # Convert numeric columns
    numeric_columns = ['keypoints1', 'keypoints2', 'matches', 'inliers', 'inlier_ratio']
    for col in numeric_columns:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce').fillna(0)

    return df

def create_metrics_analysis(df, output_dir):
    """Create comprehensive metrics analysis chart"""

    # Filter out multi-image experiments which have 0 values
    df_filtered = df[df['matches'] > 0].copy()

    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    fig.suptitle('Panorama Stitching Metrics Analysis', fontsize=16, fontweight='bold')

    # 1. Detector Comparison - Average Keypoints
    ax = axes[0, 0]
    detector_kp = df_filtered.groupby('detector')[['keypoints1', 'keypoints2']].mean()
    detector_kp.plot(kind='bar', ax=ax, color=['#3498db', '#e74c3c'])
    ax.set_title('Average Keypoints by Detector')
    ax.set_xlabel('Detector')
    ax.set_ylabel('Number of Keypoints')
    ax.set_xticklabels(ax.get_xticklabels(), rotation=0)
    ax.legend(['Image 1', 'Image 2'])
    ax.grid(True, alpha=0.3)

    # 2. Matches and Inliers by Detector
    ax = axes[0, 1]
    detector_matches = df_filtered.groupby('detector')[['matches', 'inliers']].mean()
    detector_matches.plot(kind='bar', ax=ax, color=['#2ecc71', '#f39c12'])
    ax.set_title('Average Matches and Inliers')
    ax.set_xlabel('Detector')
    ax.set_ylabel('Count')
    ax.set_xticklabels(ax.get_xticklabels(), rotation=0)
    ax.legend(['Matches', 'Inliers'])
    ax.grid(True, alpha=0.3)

    # 3. RANSAC Threshold Impact
    ax = axes[0, 2]
    ransac_data = df_filtered[df_filtered['experiment'].str.contains('RANSAC', na=False)]
    if not ransac_data.empty:
        ransac_grouped = ransac_data.groupby('threshold')['inlier_ratio'].mean()
        ransac_grouped.plot(kind='line', ax=ax, marker='o', linewidth=2, markersize=8, color='#9b59b6')
        ax.set_title('RANSAC Threshold vs Inlier Ratio')
        ax.set_xlabel('RANSAC Threshold')
        ax.set_ylabel('Average Inlier Ratio (%)')
        ax.grid(True, alpha=0.3)
    else:
        ax.text(0.5, 0.5, 'No RANSAC data', ha='center', va='center', transform=ax.transAxes)

    # 4. Success Rate by Detector
    ax = axes[1, 0]
    if 'status' in df.columns:
        success_rate = df.groupby('detector')['status'].apply(
            lambda x: (x == 'SUCCESS').sum() / len(x) * 100
        )
        success_rate.plot(kind='bar', ax=ax, color='#16a085')
        ax.set_title('Success Rate by Detector')
        ax.set_xlabel('Detector')
        ax.set_ylabel('Success Rate (%)')
        ax.set_xticklabels(ax.get_xticklabels(), rotation=0)
        ax.grid(True, alpha=0.3)
    else:
        ax.text(0.5, 0.5, 'No status data', ha='center', va='center', transform=ax.transAxes)

    # 5. Scene Comparison
    ax = axes[1, 1]
    scene_data = df_filtered.groupby('scene')['inliers'].mean()
    scene_data.plot(kind='bar', ax=ax, color='#d35400')
    ax.set_title('Average Inliers by Scene')
    ax.set_xlabel('Scene')
    ax.set_ylabel('Average Inliers')
    ax.set_xticklabels(ax.get_xticklabels(), rotation=45)
    ax.grid(True, alpha=0.3)

    # 6. Blending Mode Comparison (if available)
    ax = axes[1, 2]
    blend_data = df_filtered[df_filtered['experiment'].str.contains('blend', na=False)]
    if not blend_data.empty and 'status' in blend_data.columns:
        blend_success = blend_data.groupby('blend_mode')['status'].apply(
            lambda x: (x == 'SUCCESS').sum() / len(x) * 100
        )
        blend_success.plot(kind='bar', ax=ax, color='#34495e')
        ax.set_title('Success Rate by Blending Mode')
        ax.set_xlabel('Blending Mode')
        ax.set_ylabel('Success Rate (%)')
        ax.set_xticklabels(ax.get_xticklabels(), rotation=0)
        ax.grid(True, alpha=0.3)
    else:
        ax.text(0.5, 0.5, 'No blending data', ha='center', va='center', transform=ax.transAxes)

    plt.tight_layout()

    output_path = os.path.join(output_dir, 'metrics_analysis.png')
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    plt.close()

    print(f"Created metrics analysis chart: {output_path}")
    return 'metrics_analysis.png'

def organize_visualizations(viz_dir, output_dir):
    """Organize visualization images into output directory"""
    showcase_dir = os.path.join(output_dir, 'visualizations')
    os.makedirs(showcase_dir, exist_ok=True)

    viz_files = []
    for file in os.listdir(viz_dir):
        if file.endswith(('.jpg', '.png')):
            src = os.path.join(viz_dir, file)
            dst = os.path.join(showcase_dir, file)
            shutil.copy2(src, dst)
            viz_files.append(file)

    if not viz_files:
        print("No visualization files found")
        return

    print(f"Copied {len(viz_files)} visualization files to {showcase_dir}")
    return

def organize_results_by_scene():
    """Organize results into folders by scene."""

    os.makedirs("results_analysis/by_scene", exist_ok=True)

    scenes = ['indoor_scene', 'outdoor_scene1', 'outdoor_scene2']

    for scene in scenes:
        scene_dir = f"results_analysis/by_scene/{scene}"
        os.makedirs(scene_dir, exist_ok=True)

        # Copy scene-specific results
        for file in glob.glob(f"results/{scene}_*.jpg"):
            shutil.copy2(file, scene_dir)

        # Copy visualizations
        viz_dir = f"{scene_dir}/visualizations"
        os.makedirs(viz_dir, exist_ok=True)
        for file in glob.glob(f"results/visualizations/{scene}_*.jpg"):
            shutil.copy2(file, viz_dir)

        print(f"  ✓ Organized {scene} results")

    # Organize by experiment type
    exp_types = {
        'detector_comparison': '*_orb.jpg *_akaze.jpg *_sift.jpg',
        'ransac_analysis': '*_ransac_*.jpg',
        'blending_comparison': '*_blend_*.jpg',
        'multi_image': '*_multi_*.jpg'
    }

    for exp_type, pattern in exp_types.items():
        exp_dir = f"results_analysis/by_experiment/{exp_type}"
        os.makedirs(exp_dir, exist_ok=True)

        for pat in pattern.split():
            for file in glob.glob(f"results/{pat}"):
                shutil.copy2(file, exp_dir)

        print(f"  ✓ Organized {exp_type} results")

def main():
    """Main analysis pipeline execution."""

    print("\n" + "="*60)
    print("PANORAMA STITCHING ANALYSIS PIPELINE")
    print("="*60 + "\n")

    # Step 1: Fix CSV format if needed
    print("📊 Processing experiment metrics...")
    fix_csv_format()

    # Step 2: Load experiment data
    df = load_experiment_data()
    if df is None:
        print("❌ Failed to load experiment data")
        return

    print(f"  ✓ Loaded {len(df)} experiments")

    # Step 3: Create output directory
    output_dir = 'results_analysis'
    os.makedirs(output_dir, exist_ok=True)

    # Step 4: Generate metrics analysis
    print("\n📈 Generating metrics analysis...")
    create_metrics_analysis(df, output_dir)

    # Step 5: Organize visualizations
    if os.path.exists('results/visualizations'):
        print("\n🖼️  Organizing visualizations...")
        organize_visualizations('results/visualizations', output_dir)

    # Step 6: Copy panorama results
    print("\n📁 Copying panorama results...")
    panoramas_dir = os.path.join(output_dir, 'panoramas')
    os.makedirs(panoramas_dir, exist_ok=True)

    panorama_patterns = [
        'results/*_pair_*.jpg',
        'results/*_multi_*.jpg',
        'results/*_ransac_*.jpg',
        'results/*_blend_*.jpg'
    ]

    panorama_count = 0
    for pattern in panorama_patterns:
        for file in glob.glob(pattern):
            shutil.copy2(file, panoramas_dir)
            panorama_count += 1

    print(f"  ✓ Copied {panorama_count} panorama images")

    # Step 7: Save CSV to analysis directory
    df.to_csv(f'{output_dir}/metrics_analyzed.csv', index=False)
    print(f"  ✓ Saved analyzed metrics to {output_dir}/metrics_analyzed.csv")

    # Step 8: Organize by scene
    print("\n🗂️  Organizing results by scene and experiment type...")
    organize_results_by_scene()

    # Step 9: Print summary statistics
    print("\n" + "="*60)
    print("ANALYSIS SUMMARY")
    print("="*60)

    if 'status' in df.columns:
        success_count = (df['status'] == 'SUCCESS').sum()
        total_count = len(df)
        success_rate = (success_count / total_count * 100) if total_count > 0 else 0
        print(f"✅ Success Rate: {success_count}/{total_count} ({success_rate:.1f}%)")

    # Group by detector
    print("\n📊 Results by Detector:")
    for detector in df['detector'].unique():
        detector_df = df[df['detector'] == detector]
        if 'status' in detector_df.columns:
            success = (detector_df['status'] == 'SUCCESS').sum()
            total = len(detector_df)
            print(f"  • {detector.upper()}: {success}/{total} successful")

    print("\n" + "="*60)
    print(f"📁 Results saved to: {output_dir}/")
    print(f"📊 View metrics at: {output_dir}/metrics_analysis.png")
    print(f"🖼️  Visualizations at: {output_dir}/visualizations/")
    print(f"📷 Panoramas at: {output_dir}/panoramas/")
    print("="*60 + "\n")

if __name__ == "__main__":
    main()