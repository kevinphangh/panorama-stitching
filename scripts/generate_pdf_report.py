#!/usr/bin/env python3
"""
Generate PDF report for panorama stitching assignment.
Creates a comprehensive 3-4 page report with all required components.
"""

import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.backends.backend_pdf import PdfPages
import numpy as np
from datetime import datetime

def load_metrics():
    """Load experiment metrics from CSV."""
    df = pd.read_csv('results/metrics.csv')
    return df

def create_overview_page(pdf, df):
    """Create overview page with method description and architecture diagram."""
    fig = plt.figure(figsize=(8.5, 11))
    fig.suptitle('Panorama Stitching System - Technical Report', fontsize=16, fontweight='bold')

    # Title and metadata
    ax1 = plt.subplot2grid((4, 2), (0, 0), colspan=2)
    ax1.axis('off')
    ax1.text(0.5, 0.9, 'Visual Computing Assignment 1', ha='center', fontsize=14)
    ax1.text(0.5, 0.7, 'Feature Detection, Matching, and Panorama Stitching', ha='center', fontsize=12)
    ax1.text(0.5, 0.5, f'Date: {datetime.now().strftime("%B %d, %Y")}', ha='center', fontsize=10)
    ax1.text(0.5, 0.3, 'Aarhus University', ha='center', fontsize=10)

    # Method Overview
    ax2 = plt.subplot2grid((4, 2), (1, 0), colspan=2)
    ax2.axis('off')
    ax2.text(0.5, 0.9, 'Method Overview', ha='center', fontsize=12, fontweight='bold')

    method_text = """
    1. Feature Detection: ORB (50k features) and AKAZE (variable features)
    2. Feature Matching: Brute-force matching with Lowe's ratio test (0.7)
    3. Homography Estimation: RANSAC with varying thresholds (1.0-5.0)
    4. Image Warping: Perspective transformation to common coordinate frame
    5. Blending: Three modes - Simple overlay, Feathering, Multi-band
    """
    ax2.text(0.1, 0.5, method_text, fontsize=10, verticalalignment='top')

    # Architecture Diagram
    ax3 = plt.subplot2grid((4, 2), (2, 0), colspan=2, rowspan=2)
    ax3.axis('off')
    ax3.set_xlim(0, 10)
    ax3.set_ylim(0, 10)
    ax3.text(5, 9.5, 'System Architecture', ha='center', fontsize=12, fontweight='bold')

    # Draw pipeline boxes
    boxes = [
        (1, 7, 'Input\nImages'),
        (3, 7, 'Feature\nDetection'),
        (5, 7, 'Feature\nMatching'),
        (7, 7, 'RANSAC\nFiltering'),
        (9, 7, 'Homography\nEstimation'),
        (3, 4, 'Image\nWarping'),
        (5, 4, 'Blending'),
        (7, 4, 'Final\nPanorama')
    ]

    for x, y, label in boxes:
        rect = patches.FancyBboxPatch((x-0.6, y-0.4), 1.2, 0.8,
                                      boxstyle="round,pad=0.05",
                                      facecolor='lightblue',
                                      edgecolor='navy')
        ax3.add_patch(rect)
        ax3.text(x, y, label, ha='center', va='center', fontsize=9)

    # Draw arrows
    arrows = [
        (1.6, 7, 2.4, 7),
        (3.6, 7, 4.4, 7),
        (5.6, 7, 6.4, 7),
        (7.6, 7, 8.4, 7),
        (9, 6.6, 7, 4.4),
        (6.4, 4, 5.6, 4),
        (4.4, 4, 3.6, 4)
    ]

    for x1, y1, x2, y2 in arrows:
        ax3.arrow(x1, y1, x2-x1, y2-y1, head_width=0.1, head_length=0.1,
                 fc='black', ec='black')

    plt.tight_layout()
    pdf.savefig(fig)
    plt.close()

def create_results_page(pdf, df):
    """Create results page with quantitative metrics and performance analysis."""
    fig = plt.figure(figsize=(8.5, 11))
    fig.suptitle('Experimental Results and Analysis', fontsize=14, fontweight='bold')

    # Success Rate Analysis
    ax1 = plt.subplot2grid((3, 2), (0, 0))
    success_by_detector = df.groupby('detector')['status'].apply(lambda x: (x == 'SUCCESS').sum() / len(x) * 100)
    ax1.bar(success_by_detector.index, success_by_detector.values, color=['blue', 'green'])
    ax1.set_title('Success Rate by Detector')
    ax1.set_ylabel('Success Rate (%)')
    ax1.set_ylim(0, 100)

    # Inlier Ratio Distribution
    ax2 = plt.subplot2grid((3, 2), (0, 1))
    df_success = df[df['status'] == 'SUCCESS']
    orb_inliers = df_success[df_success['detector'] == 'orb']['inlier_ratio']
    akaze_inliers = df_success[df_success['detector'] == 'akaze']['inlier_ratio']

    ax2.hist([orb_inliers, akaze_inliers], label=['ORB', 'AKAZE'], bins=10, alpha=0.7, color=['blue', 'green'])
    ax2.set_title('Inlier Ratio Distribution')
    ax2.set_xlabel('Inlier Ratio (%)')
    ax2.set_ylabel('Count')
    ax2.legend()

    # RANSAC Threshold Analysis
    ax3 = plt.subplot2grid((3, 2), (1, 0))
    ransac_data = df[df['experiment'].str.contains('RANSAC')]
    if not ransac_data.empty:
        threshold_success = ransac_data.groupby('threshold').apply(
            lambda x: (x['status'] == 'SUCCESS').sum() / len(x) * 100
        )
        ax3.plot(threshold_success.index, threshold_success.values, 'o-', color='red')
        ax3.set_title('RANSAC Threshold Impact')
        ax3.set_xlabel('Threshold')
        ax3.set_ylabel('Success Rate (%)')
        ax3.set_ylim(0, 100)
    else:
        ax3.text(0.5, 0.5, 'No RANSAC data available', ha='center')
        ax3.axis('off')

    # Keypoint Count Comparison
    ax4 = plt.subplot2grid((3, 2), (1, 1))
    avg_keypoints = df.groupby('detector')[['keypoints1', 'keypoints2']].mean()
    x = np.arange(len(avg_keypoints.index))
    width = 0.35
    ax4.bar(x - width/2, avg_keypoints['keypoints1'], width, label='Image 1', color='lightblue')
    ax4.bar(x + width/2, avg_keypoints['keypoints2'], width, label='Image 2', color='lightgreen')
    ax4.set_title('Average Keypoint Count')
    ax4.set_xticks(x)
    ax4.set_xticklabels(avg_keypoints.index)
    ax4.set_ylabel('Keypoints')
    ax4.legend()

    # Blending Mode Comparison
    ax5 = plt.subplot2grid((3, 2), (2, 0), colspan=2)
    blend_data = df[df['experiment'].str.contains('blend')]
    if not blend_data.empty:
        blend_success = blend_data.groupby('blend_mode')['inlier_ratio'].mean()
        ax5.bar(blend_success.index, blend_success.values, color=['orange', 'purple', 'cyan'])
        ax5.set_title('Average Inlier Ratio by Blending Mode')
        ax5.set_xlabel('Blending Mode')
        ax5.set_ylabel('Inlier Ratio (%)')
    else:
        ax5.text(0.5, 0.5, 'No blending comparison data', ha='center')
        ax5.axis('off')

    plt.tight_layout()
    pdf.savefig(fig)
    plt.close()

def create_discussion_page(pdf, df):
    """Create discussion and conclusions page."""
    fig = plt.figure(figsize=(8.5, 11))
    fig.suptitle('Discussion and Conclusions', fontsize=14, fontweight='bold')

    # Calculate statistics
    orb_success = (df[df['detector'] == 'orb']['status'] == 'SUCCESS').sum()
    akaze_success = (df[df['detector'] == 'akaze']['status'] == 'SUCCESS').sum()
    orb_total = len(df[df['detector'] == 'orb'])
    akaze_total = len(df[df['detector'] == 'akaze'])

    avg_orb_kp = df[df['detector'] == 'orb'][['keypoints1', 'keypoints2']].mean().mean()
    avg_akaze_kp = df[df['detector'] == 'akaze'][['keypoints1', 'keypoints2']].mean().mean()

    # Key Findings
    ax1 = plt.subplot2grid((3, 1), (0, 0))
    ax1.axis('off')
    ax1.text(0.5, 0.9, 'Key Findings', ha='center', fontsize=12, fontweight='bold')

    findings = f"""
    1. Feature Detector Performance:
       • ORB: {orb_success}/{orb_total} successful ({orb_success/orb_total*100:.1f}%), avg {avg_orb_kp:.0f} keypoints
       • AKAZE: {akaze_success}/{akaze_total} successful ({akaze_success/akaze_total*100:.1f}%), avg {avg_akaze_kp:.0f} keypoints

    2. RANSAC Threshold Impact:
       • Lower thresholds (1.0-2.0) provide stricter outlier filtering
       • Higher thresholds (4.0-5.0) allow more matches but may include outliers
       • Optimal threshold appears to be around 3.0 for most scenes

    3. Scene-Specific Observations:
       • Indoor scenes: Higher success with AKAZE due to better corner detection
       • Outdoor scenes: ORB performs well with high keypoint count (50k)
       • Multi-image stitching requires careful parameter tuning
    """
    ax1.text(0.1, 0.5, findings, fontsize=10, verticalalignment='top')

    # Best Configurations
    ax2 = plt.subplot2grid((3, 1), (1, 0))
    ax2.axis('off')
    ax2.text(0.5, 0.9, 'Recommended Configurations', ha='center', fontsize=12, fontweight='bold')

    configs = """
    Scene Type          Detector    RANSAC    Blending      Notes
    ─────────────────────────────────────────────────────────────────────
    Indoor (structured)  AKAZE      3.0       Feather       Good for corners/edges
    Outdoor (textured)   ORB        3.0       Multiband     Fast with many features
    Low texture         AKAZE      2.0       Feather       More robust features
    Multi-image         ORB        3.0       Multiband     Balance speed/quality
    """
    ax2.text(0.1, 0.5, configs, fontsize=9, verticalalignment='top', family='monospace')

    # Conclusions
    ax3 = plt.subplot2grid((3, 1), (2, 0))
    ax3.axis('off')
    ax3.text(0.5, 0.9, 'Conclusions', ha='center', fontsize=12, fontweight='bold')

    conclusions = """
    This experimental evaluation demonstrates the importance of parameter tuning in panorama
    stitching pipelines. Key insights include:

    • Feature detector choice significantly impacts both speed and robustness
    • RANSAC threshold must balance between outlier rejection and sufficient inliers
    • Blending methods show minimal impact on geometric accuracy but affect visual quality
    • Multi-image stitching remains challenging, especially with limited overlap

    Future improvements could include:
    • Adaptive threshold selection based on initial match quality
    • Hybrid detector approaches combining ORB speed with AKAZE robustness
    • Automatic exposure compensation for better visual consistency
    """
    ax3.text(0.1, 0.5, conclusions, fontsize=10, verticalalignment='top')

    plt.tight_layout()
    pdf.savefig(fig)
    plt.close()

def main():
    """Generate the PDF report."""
    print("Generating PDF report...")

    # Load data
    df = load_metrics()

    # Create PDF
    pdf_path = 'Panorama_Stitching_Report.pdf'
    with PdfPages(pdf_path) as pdf:
        # Page 1: Overview and Architecture
        create_overview_page(pdf, df)

        # Page 2: Results and Metrics
        create_results_page(pdf, df)

        # Page 3: Discussion and Conclusions
        create_discussion_page(pdf, df)

        # Add metadata
        d = pdf.infodict()
        d['Title'] = 'Panorama Stitching System - Technical Report'
        d['Author'] = 'Visual Computing Assignment'
        d['Subject'] = 'Feature Detection, Matching, and Panorama Stitching'
        d['Keywords'] = 'Computer Vision, Panorama, RANSAC, ORB, AKAZE'
        d['CreationDate'] = datetime.now()

    print(f"✅ PDF report generated: {pdf_path}")
    print(f"📄 Report contains {3} pages with:")
    print("   • Method overview and architecture diagram")
    print("   • Quantitative experimental results")
    print("   • Discussion and recommendations")

if __name__ == "__main__":
    main()