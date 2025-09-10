#!/usr/bin/env python3

"""
Plot experiment results from metrics.csv
Generates publication-quality plots for the report
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import os

def main():
    # Check if metrics file exists
    metrics_file = 'results/metrics.csv'
    if not os.path.exists(metrics_file):
        print(f"Error: {metrics_file} not found. Run experiments first.")
        sys.exit(1)
    
    # Read metrics
    df = pd.read_csv(metrics_file)
    
    # Set style for publication quality
    sns.set_style("whitegrid")
    plt.rcParams['figure.dpi'] = 150
    plt.rcParams['savefig.dpi'] = 300
    
    # Create figure with subplots
    fig, axes = plt.subplots(2, 2, figsize=(12, 10))
    
    # 1. Detection time comparison
    ax = axes[0, 0]
    detector_times = df.groupby('detector')['detection_time_ms'].mean()
    detector_times.plot(kind='bar', ax=ax, color=['#1f77b4', '#ff7f0e'])
    ax.set_title('Average Feature Detection Time')
    ax.set_ylabel('Time (ms)')
    ax.set_xlabel('Detector')
    ax.set_xticklabels(ax.get_xticklabels(), rotation=0)
    
    # 2. Inlier count by RANSAC threshold
    ax = axes[0, 1]
    threshold_groups = df.groupby('ransac_threshold')['num_inliers'].mean()
    threshold_groups.plot(marker='o', ax=ax, linewidth=2)
    ax.set_title('Inliers vs RANSAC Threshold')
    ax.set_xlabel('RANSAC Threshold')
    ax.set_ylabel('Average Number of Inliers')
    ax.grid(True, alpha=0.3)
    
    # 3. Blending mode performance
    ax = axes[1, 0]
    blend_times = df.groupby('blend_mode')['total_time_ms'].mean()
    blend_times.plot(kind='bar', ax=ax, color=['#2ca02c', '#d62728', '#9467bd'])
    ax.set_title('Processing Time by Blending Mode')
    ax.set_ylabel('Total Time (ms)')
    ax.set_xlabel('Blending Mode')
    ax.set_xticklabels(ax.get_xticklabels(), rotation=45)
    
    # 4. Success rate comparison
    ax = axes[1, 1]
    success_data = df.groupby(['detector', 'blend_mode'])['success'].mean() * 100
    success_pivot = success_data.unstack()
    success_pivot.plot(kind='bar', ax=ax)
    ax.set_title('Stitching Success Rate')
    ax.set_ylabel('Success Rate (%)')
    ax.set_xlabel('Detector')
    ax.set_xticklabels(ax.get_xticklabels(), rotation=0)
    ax.legend(title='Blend Mode', loc='lower right')
    
    plt.tight_layout()
    
    # Save figure
    output_path = 'results/experiment_plots.png'
    plt.savefig(output_path, bbox_inches='tight')
    print(f"Plots saved to {output_path}")
    
    # Also save individual plots for the report
    for i, (title, data) in enumerate([
        ('detection_times', detector_times),
        ('ransac_analysis', threshold_groups),
        ('blending_comparison', blend_times)
    ]):
        fig_single = plt.figure(figsize=(6, 4))
        if i < 2:
            data.plot(kind='bar' if i == 0 else 'line', marker='o' if i == 1 else None)
        else:
            data.plot(kind='bar')
        plt.title(title.replace('_', ' ').title())
        plt.tight_layout()
        plt.savefig(f'results/{title}.png', bbox_inches='tight')
        plt.close()
    
    print("Individual plots saved to results/")
    plt.show()

if __name__ == '__main__':
    main()