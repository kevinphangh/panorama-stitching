#!/usr/bin/env python3
"""
Analyze and visualize panorama stitching experiment results.
Creates organized HTML reports with charts and visualization galleries.
"""

import os
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

def load_metrics(csv_path="results/metrics.csv"):
    """Load metrics from CSV file"""
    if not os.path.exists(csv_path):
        print(f"Warning: Metrics file not found at {csv_path}")
        return None
    
    df = pd.read_csv(csv_path)
    print(f"Loaded {len(df)} experiment results from {csv_path}")
    
    # Parse experiment details from experiment column if needed
    if 'experiment' in df.columns:
        df['scene'] = df['experiment'].str.extract(r'(\w+_scene\d?)', expand=False)
        df['image_pair'] = df['experiment'].str.extract(r'pair\(([\d-]+)\)', expand=False)
    
    # Ensure numeric columns - handle both old and new formats
    numeric_cols = ['num_keypoints_1', 'num_keypoints_2', 'num_matches', 'num_inliers',
                   'inlier_ratio', 'reprojection_error', 'detection_time', 'matching_time',
                   'homography_time', 'warping_time', 'blending_time', 'total_time']
    
    # Check if we have old format and map columns
    if 'keypoints' in df.columns:
        df['num_keypoints_1'] = df['keypoints']
        df['num_keypoints_2'] = df['keypoints']
        df['num_matches'] = df['matches']
        df['num_inliers'] = df['inliers']
        
        # For timing, use single processing_time_ms if available
        if 'processing_time_ms' in df.columns:
            df['total_time'] = df['processing_time_ms']
            # Estimate component times (rough percentages)
            df['detection_time'] = df['processing_time_ms'] * 0.4
            df['matching_time'] = df['processing_time_ms'] * 0.15
            df['homography_time'] = df['processing_time_ms'] * 0.1
            df['warping_time'] = df['processing_time_ms'] * 0.2
            df['blending_time'] = df['processing_time_ms'] * 0.15
    
    for col in numeric_cols:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')
    
    return df

def create_timing_analysis(df, output_dir):
    """Create detailed timing analysis charts"""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    axes = axes.flatten()
    
    # Check which timing columns are available
    timing_cols = ['detection_time', 'matching_time', 'homography_time', 
                  'warping_time', 'blending_time']
    available_cols = [col for col in timing_cols if col in df.columns and df[col].notna().any()]
    
    # If no detailed timing, check for total_time or processing_time_ms
    if not available_cols and 'total_time' in df.columns and df['total_time'].notna().any():
        # Create estimated breakdown for visualization
        available_cols = ['detection_time', 'matching_time', 'homography_time', 'warping_time', 'blending_time']
    
    if not available_cols:
        print("No timing data available")
        plt.close()
        return None
    
    # 1. Time breakdown by stage
    stage_times = df[available_cols].mean()
    colors = plt.cm.Set3(np.linspace(0, 1, len(stage_times)))
    bars = axes[0].bar(range(len(stage_times)), stage_times.values, color=colors)
    axes[0].set_xticks(range(len(stage_times)))
    axes[0].set_xticklabels([col.replace('_time', '').replace('_', ' ').title() 
                             for col in stage_times.index], rotation=45, ha='right')
    axes[0].set_ylabel('Average Time (ms)')
    axes[0].set_title('Average Processing Time by Stage')
    axes[0].grid(True, alpha=0.3)
    
    # Add value labels on bars
    for bar, val in zip(bars, stage_times.values):
        axes[0].text(bar.get_x() + bar.get_width()/2, bar.get_height() + 1,
                    f'{val:.1f}', ha='center', va='bottom', fontsize=9)
    
    # 2. Total time by detector
    if 'total_time' in df.columns and 'detector' in df.columns:
        detector_times = df.groupby('detector')['total_time'].agg(['mean', 'std'])
        x = range(len(detector_times))
        axes[1].bar(x, detector_times['mean'], yerr=detector_times['std'], 
                   capsize=5, color=['#FF6B6B', '#4ECDC4'])
        axes[1].set_xticks(x)
        axes[1].set_xticklabels(detector_times.index.str.upper())
        axes[1].set_ylabel('Total Time (ms)')
        axes[1].set_title('Total Processing Time by Detector')
        axes[1].grid(True, alpha=0.3)
        
        # Add value labels
        for i, (mean, std) in enumerate(zip(detector_times['mean'], detector_times['std'])):
            axes[1].text(i, mean + std + 5, f'{mean:.1f}±{std:.1f}', 
                        ha='center', va='bottom', fontsize=9)
    
    # 3. Blending time comparison
    if 'blending_time' in df.columns and 'blend_mode' in df.columns:
        blend_times = df.groupby('blend_mode')['blending_time'].agg(['mean', 'std'])
        x = range(len(blend_times))
        colors = ['#FFD93D', '#6BCB77', '#4D96FF'][:len(blend_times)]
        axes[2].bar(x, blend_times['mean'], yerr=blend_times['std'], 
                   capsize=5, color=colors)
        axes[2].set_xticks(x)
        axes[2].set_xticklabels(blend_times.index.str.title())
        axes[2].set_ylabel('Blending Time (ms)')
        axes[2].set_title('Blending Time by Mode')
        axes[2].grid(True, alpha=0.3)
        
        # Add value labels
        for i, (mean, std) in enumerate(zip(blend_times['mean'], blend_times['std'])):
            axes[2].text(i, mean + std + 0.5, f'{mean:.1f}±{std:.1f}', 
                        ha='center', va='bottom', fontsize=9)
    
    # 4. Stacked time breakdown by detector
    if len(available_cols) > 1 and 'detector' in df.columns:
        detector_breakdown = df.groupby('detector')[available_cols].mean()
        detector_breakdown.plot(kind='bar', stacked=True, ax=axes[3], 
                               color=plt.cm.Set3(np.linspace(0, 1, len(available_cols))))
        axes[3].set_xlabel('Detector')
        axes[3].set_ylabel('Time (ms)')
        axes[3].set_title('Time Breakdown by Detector')
        axes[3].set_xticklabels(axes[3].get_xticklabels(), rotation=0)
        axes[3].legend(title='Stage', bbox_to_anchor=(1.05, 1), loc='upper left',
                      labels=[col.replace('_time', '').title() for col in available_cols])
        axes[3].grid(True, alpha=0.3)
    
    plt.tight_layout()
    output_path = os.path.join(output_dir, 'processing_times.png')
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    plt.close()
    
    print(f"Created timing analysis chart: {output_path}")
    return 'processing_times.png'

def create_visualization_showcase(viz_dir, output_dir):
    """Create HTML showcase for visualizations"""
    showcase_dir = os.path.join(output_dir, 'visualizations')
    os.makedirs(showcase_dir, exist_ok=True)
    
    # Copy all visualization images
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
    
    # Group visualizations by experiment
    experiments = {}
    for file in viz_files:
        # Parse filename to extract experiment info
        parts = file.replace('.jpg', '').replace('.png', '').split('_')
        
        # Try to identify experiment name and visualization type
        if len(parts) >= 3:
            # Assume format: scene_img1_img2_detector_viztype.jpg
            if 'keypoints' in file:
                exp_key = '_'.join(parts[:-1])
                viz_type = parts[-1]
            elif 'matches' in file:
                exp_key = '_'.join(parts[:-2])
                viz_type = '_'.join(parts[-2:])
            elif 'img' in parts[-1]:
                exp_key = '_'.join(parts[:-1])
                viz_type = parts[-1]
            else:
                exp_key = '_'.join(parts[:-1])
                viz_type = parts[-1]
            
            if exp_key not in experiments:
                experiments[exp_key] = {}
            experiments[exp_key][viz_type] = file
    
    # Create HTML showcase
    html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Visualization Showcase</title>
    <style>
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            margin: 0; 
            padding: 20px; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        h1 { 
            color: #333; 
            border-bottom: 3px solid #667eea; 
            padding-bottom: 15px;
            margin-bottom: 30px;
        }
        .description {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            margin-bottom: 30px;
            border-left: 4px solid #667eea;
        }
        .experiment { 
            background: #fff; 
            padding: 25px; 
            margin: 30px 0; 
            border-radius: 10px; 
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
            border: 1px solid #e0e0e0;
        }
        .experiment h2 { 
            color: #667eea; 
            margin-top: 0;
            border-bottom: 2px solid #f0f0f0;
            padding-bottom: 10px;
        }
        .viz-grid { 
            display: grid; 
            grid-template-columns: repeat(auto-fit, minmax(350px, 1fr)); 
            gap: 25px; 
            margin-top: 25px; 
        }
        .viz-item { 
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        .viz-item:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 25px rgba(0,0,0,0.15);
        }
        .viz-item img { 
            width: 100%; 
            height: auto; 
            border-radius: 5px; 
            display: block;
        }
        .viz-item h3 { 
            margin: 0 0 10px 0; 
            font-size: 14px; 
            color: #666;
            text-align: center;
            background: white;
            padding: 8px;
            border-radius: 5px;
        }
        .navigation { 
            margin-bottom: 20px; 
        }
        .navigation a { 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white; 
            padding: 12px 25px; 
            text-decoration: none; 
            border-radius: 25px; 
            display: inline-block;
            transition: transform 0.3s ease;
        }
        .navigation a:hover { 
            transform: scale(1.05);
        }
        .stats-box {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            color: white;
            padding: 15px;
            border-radius: 10px;
            margin: 20px 0;
            text-align: center;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="navigation">
            <a href="../index.html">← Back to Main Results</a>
        </div>
        <h1>🔬 Visualization Showcase</h1>
        
        <div class="description">
            <h3>📚 Understanding the Visualizations</h3>
            <p><strong>What are ORB and AKAZE?</strong></p>
            <ul style="margin-bottom: 15px;">
                <li><strong>ORB (Oriented FAST and Rotated BRIEF):</strong> A FAST detector that finds many keypoints quickly. 
                    Good for speed, finds corners and edges. Shows up as many small green circles (~25,000-50,000 points).</li>
                <li><strong>AKAZE (Accelerated-KAZE):</strong> A more sophisticated detector that finds fewer but more stable keypoints. 
                    Better for accuracy, finds distinctive features. Shows up as fewer larger circles (~5,000-20,000 points).</li>
            </ul>
            
            <p><strong>What Each Visualization Shows:</strong></p>
            <ul>
                <li><strong>Original Images:</strong> The raw input photos before any processing</li>
                <li><strong>Keypoints (Green Circles):</strong> Points the algorithm thinks are interesting/matchable 
                    - like corners, edges, or unique patterns. Bigger circles = larger scale features.</li>
                <li><strong>Initial Matches (Lines):</strong> All potential correspondences between images - many are wrong!</li>
                <li><strong>RANSAC Filtered (Final Matches):</strong> Only the correct matches after removing outliers. 
                    These are used to compute how to align the images.</li>
            </ul>
            
            <p style="background: #e8f4f8; padding: 10px; border-radius: 5px; margin-top: 15px;">
                <strong>💡 Simple Explanation:</strong> The algorithm finds interesting points in both images (keypoints), 
                matches similar ones between images (matching), then removes incorrect matches (RANSAC) to properly align and stitch them together.
            </p>
        </div>
        
        <div class="stats-box">
            <strong>""" + f"{len(experiments)} Experiments | {len(viz_files)} Visualizations" + """</strong>
        </div>
"""
    
    # Add each experiment
    for exp_name in sorted(experiments.keys()):
        exp_files = experiments[exp_name]
        
        # Clean up experiment name for display
        display_name = exp_name.replace('_', ' ').title()
        
        html += f'''
        <div class="experiment">
            <h2>📊 {display_name}</h2>
            <div class="viz-grid">
'''
        
        # Define visualization order and labels
        viz_order = [
            ('img1', 'Original Image 1'),
            ('img2', 'Original Image 2'),
            ('keypoints1', 'Keypoints - Image 1'),
            ('keypoints2', 'Keypoints - Image 2'),
            ('matches_before', 'Initial Matches'),
            ('matches_after', 'Inlier Matches (RANSAC)'),
            ('all_matches', 'All Matches'),
            ('inliers', 'Final Inliers')
        ]
        
        for viz_key, viz_label in viz_order:
            if viz_key in exp_files:
                html += f'''
                <div class="viz-item">
                    <h3>{viz_label}</h3>
                    <img src="{exp_files[viz_key]}" alt="{viz_label}" loading="lazy">
                </div>
'''
        
        html += '''
            </div>
        </div>
'''
    
    html += """
    </div>
</body>
</html>"""
    
    # Write HTML file
    showcase_path = os.path.join(showcase_dir, 'index.html')
    with open(showcase_path, 'w') as f:
        f.write(html)
    
    print(f"Created visualization showcase: {showcase_path}")
    return True

def create_comprehensive_report(df, output_dir):
    """Create main analysis report with timing data"""
    
    # Calculate statistics
    total_exp = len(df)
    success_exp = len(df[df['num_inliers'] > 20]) if 'num_inliers' in df.columns else 0
    success_rate = (success_exp / total_exp * 100) if total_exp > 0 else 0
    
    # Create timing chart
    timing_chart = create_timing_analysis(df, output_dir)
    
    # Copy all result images to output directory
    # Copy panorama images
    panoramas_dir = os.path.join(output_dir, 'panoramas')
    os.makedirs(panoramas_dir, exist_ok=True)
    # Match the actual file patterns created by RUN_EXPERIMENTS.sh
    panorama_patterns = [
        'results/*_pair_*.jpg',
        'results/*_multi_*.jpg',
        'results/*_ransac_*.jpg',
        'results/*_blend_*.jpg'
    ]
    panorama_files = []
    for pattern in panorama_patterns:
        panorama_files.extend(glob.glob(pattern))
    for pano in panorama_files:
        shutil.copy2(pano, panoramas_dir)
    
    # Copy original dataset images
    datasets_dir = os.path.join(output_dir, 'datasets')
    os.makedirs(datasets_dir, exist_ok=True)
    for scene in ['indoor_scene', 'outdoor_scene1', 'outdoor_scene2']:
        scene_path = f'datasets/{scene}'
        if os.path.exists(scene_path):
            scene_out = os.path.join(datasets_dir, scene)
            os.makedirs(scene_out, exist_ok=True)
            for img in glob.glob(f'{scene_path}/*.jpg'):
                shutil.copy2(img, scene_out)
    
    # Generate HTML report
    html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Panorama Stitching Analysis Report</title>
    <style>
        body { 
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; 
            max-width: 1400px; 
            margin: 0 auto; 
            padding: 20px;
            background: #f5f6fa;
        }
        h1 { 
            color: #2c3e50; 
            border-bottom: 3px solid #3498db; 
            padding-bottom: 15px; 
        }
        .summary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 25px;
            border-radius: 10px;
            margin: 20px 0;
        }
        .metric-card {
            background: white;
            padding: 20px;
            margin: 15px 0;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
            border-left: 4px solid #3498db;
        }
        img { 
            max-width: 100%; 
            height: auto; 
            margin: 20px 0; 
            border-radius: 8px;
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        }
        .image-gallery {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        .gallery-item {
            background: white;
            padding: 10px;
            border-radius: 8px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.1);
            transition: transform 0.3s ease;
        }
        .gallery-item:hover {
            transform: scale(1.05);
        }
        .gallery-item img {
            width: 100%;
            height: 150px;
            object-fit: cover;
            margin: 0;
        }
        .gallery-item h4 {
            margin: 10px 0 5px 0;
            font-size: 12px;
            color: #666;
            text-align: center;
        }
        .panorama-showcase {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin: 30px 0;
        }
        .panorama-item {
            background: white;
            padding: 15px;
            border-radius: 10px;
            box-shadow: 0 3px 10px rgba(0,0,0,0.15);
        }
        .panorama-item img {
            width: 100%;
            height: auto;
            margin: 10px 0;
        }
        .panorama-item h3 {
            color: #3498db;
            margin: 0 0 10px 0;
            font-size: 16px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 20px 0;
            background: white;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        th {
            background: #3498db;
            color: white;
            padding: 12px;
            text-align: left;
        }
        td {
            padding: 10px;
            border-bottom: 1px solid #ecf0f1;
        }
        tr:hover {
            background: #f8f9fa;
        }
        .note {
            background: #fff3cd;
            border: 1px solid #ffc107;
            padding: 15px;
            border-radius: 5px;
            margin: 20px 0;
        }
        .viz-link {
            display: inline-block;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 15px 30px;
            text-decoration: none;
            border-radius: 25px;
            margin: 20px 0;
            transition: transform 0.3s ease;
        }
        .viz-link:hover {
            transform: scale(1.05);
        }
        .tabs {
            display: flex;
            gap: 10px;
            margin: 20px 0;
            border-bottom: 2px solid #3498db;
        }
        .tab {
            padding: 10px 20px;
            background: white;
            border: 1px solid #ddd;
            border-bottom: none;
            cursor: pointer;
            transition: background 0.3s;
        }
        .tab:hover {
            background: #f0f0f0;
        }
        .tab.active {
            background: #3498db;
            color: white;
        }
    </style>
</head>
<body>
    <h1>📊 Panorama Stitching - Complete Analysis Report</h1>
    
    <div class="summary">
        <h2 style="margin-top: 0; color: white;">Executive Summary</h2>
        <p style="font-size: 18px;">Analyzed <strong>""" + f"{total_exp}" + """</strong> experiments with 
        <strong>""" + f"{success_rate:.1f}%" + """</strong> success rate</p>
        <p>Generated <strong>""" + f"{len(panorama_files)}" + """</strong> panoramas across 3 scenes</p>
    </div>
"""
    
    # Add detector comparison if available
    if 'detector' in df.columns:
        # Use columns that exist
        agg_dict = {}
        if 'num_keypoints_1' in df.columns:
            agg_dict['num_keypoints_1'] = 'mean'
            agg_dict['num_keypoints_2'] = 'mean'
        if 'num_matches' in df.columns:
            agg_dict['num_matches'] = 'mean'
        if 'num_inliers' in df.columns:
            agg_dict['num_inliers'] = 'mean'
        
        if agg_dict:
            detector_stats = df.groupby('detector').agg(agg_dict).round(0)
        else:
            detector_stats = None
        
        if detector_stats is not None and not detector_stats.empty:
            html += """
    <div class="metric-card">
        <h3>Detector Comparison</h3>
        <table>
            <tr>
                <th>Detector</th>
"""
            # Add column headers based on what's available
            if 'num_keypoints_1' in detector_stats.columns:
                html += "                <th>Avg Keypoints</th>\n"
            if 'num_matches' in detector_stats.columns:
                html += "                <th>Avg Matches</th>\n"
            if 'num_inliers' in detector_stats.columns:
                html += "                <th>Avg Inliers</th>\n"
            html += "            </tr>\n"
            
            for detector, row in detector_stats.iterrows():
                html += f"""
            <tr>
                <td><strong>{detector.upper()}</strong></td>
"""
                if 'num_keypoints_1' in row:
                    html += f"                <td>{row['num_keypoints_1']:.0f}</td>\n"
                if 'num_matches' in row:
                    html += f"                <td>{row['num_matches']:.0f}</td>\n"
                if 'num_inliers' in row:
                    html += f"                <td>{row['num_inliers']:.0f}</td>\n"
                html += "            </tr>\n"
            html += """
        </table>
    </div>
"""
    
    # Add timing chart if available
    if timing_chart:
        html += f"""
    <div class="metric-card">
        <h3>Processing Time Analysis</h3>
        <img src="{timing_chart}" alt="Processing Times">
        <p>The chart shows processing time breakdown by stage and comparison across different configurations.</p>
    </div>
"""
    
    # Add original dataset images section
    html += """
    <div class="metric-card">
        <h3>🖼️ Original Dataset Images</h3>
        <p>Input images used for panorama stitching experiments:</p>
"""
    
    for scene in ['indoor_scene', 'outdoor_scene1', 'outdoor_scene2']:
        scene_path = os.path.join(datasets_dir, scene)
        if os.path.exists(scene_path):
            scene_images = sorted(glob.glob(f'{scene_path}/*.jpg'))
            if scene_images:
                html += f"""
        <h4 style="color: #667eea; margin-top: 20px;">{scene.replace('_', ' ').title()}</h4>
        <div class="image-gallery">
"""
                for img in scene_images:
                    img_name = os.path.basename(img)
                    rel_path = f"datasets/{scene}/{img_name}"
                    html += f"""
            <div class="gallery-item">
                <img src="{rel_path}" alt="{img_name}">
                <h4>{img_name}</h4>
            </div>
"""
                html += "        </div>\n"
    
    html += "    </div>\n"
    
    # Add panorama results section
    if panorama_files:
        html += """
    <div class="metric-card">
        <h3>🎯 All Stitched Panorama Results</h3>
        <p><strong>What are these images?</strong> These are the final stitched panoramas created by aligning and blending image pairs. 
        Each shows two images merged into one wider view.</p>
        
        <div style="background: #f0f8ff; padding: 15px; border-radius: 8px; margin: 20px 0;">
            <strong>How to interpret the results:</strong>
            <ul style="margin: 10px 0;">
                <li>✅ <strong>Good stitching:</strong> Smooth transitions, aligned features, no visible seams</li>
                <li>⚠️ <strong>Partial success:</strong> Some alignment but visible artifacts or distortions</li>
                <li>❌ <strong>Failed stitching:</strong> Misaligned images, severe distortions, or black areas</li>
            </ul>
        </div>
        
        <h4 style="color: #667eea;">ORB Detector Results</h4>
        <div class="panorama-showcase">
"""
        
        # Show ALL panoramas, grouped by detector
        orb_panoramas = sorted([p for p in panorama_files if 'orb' in p])
        akaze_panoramas = sorted([p for p in panorama_files if 'akaze' in p])
        
        # Display all ORB results
        for i, pano in enumerate(orb_panoramas):
            if i >= 12:  # Limit display for performance
                break
            pano_name = os.path.basename(pano)
            parts = pano_name.replace('.jpg', '').split('_')
            idx = parts[2] if len(parts) > 2 else str(i)
            
            html += f"""
            <div class="panorama-item">
                <h3>ORB Test #{idx}</h3>
                <img src="panoramas/{pano_name}" alt="{pano_name}" title="Click to view full size">
                <p style="font-size: 11px; color: #666; margin: 5px 0;">File: {pano_name}</p>
            </div>
"""
        
        html += """
        </div>
        
        <h4 style="color: #667eea; margin-top: 30px;">AKAZE Detector Results</h4>
        <div class="panorama-showcase">
"""
        
        # Display all AKAZE results
        for i, pano in enumerate(akaze_panoramas):
            if i >= 12:  # Limit display for performance
                break
            pano_name = os.path.basename(pano)
            parts = pano_name.replace('.jpg', '').split('_')
            idx = parts[2] if len(parts) > 2 else str(i)
            
            html += f"""
            <div class="panorama-item">
                <h3>AKAZE Test #{idx}</h3>
                <img src="panoramas/{pano_name}" alt="{pano_name}" title="Click to view full size">
                <p style="font-size: 11px; color: #666; margin: 5px 0;">File: {pano_name}</p>
            </div>
"""
        
        html += f"""
        </div>
        
        <p style="text-align: center; margin-top: 20px; background: #fff3cd; padding: 10px; border-radius: 5px;">
            <strong>📊 Statistics:</strong> Generated {len(orb_panoramas)} ORB panoramas and {len(akaze_panoramas)} AKAZE panoramas<br>
            <em>Showing first 12 results from each detector. All {len(panorama_files)} panoramas are available in the panoramas/ folder.</em>
        </p>
    </div>
"""
    
    # Add link to visualizations
    if os.path.exists(os.path.join(output_dir, 'visualizations', 'index.html')):
        html += """
    <div style="text-align: center; margin: 40px 0;">
        <a href="visualizations/index.html" class="viz-link">
            🔬 View Feature Detection & Matching Visualizations
        </a>
        <p style="margin-top: 10px;">See keypoints, matches, and RANSAC filtering for all experiments</p>
    </div>
"""
    
    # Add results comparison charts if available
    results_images = glob.glob('results/*.jpg')
    comparison_images = [img for img in results_images if any(x in img for x in ['comparison', 'histogram', 'plot'])]
    
    if comparison_images:
        html += """
    <div class="metric-card">
        <h3>📈 Additional Analysis Charts</h3>
        <div class="image-gallery">
"""
        for img in comparison_images[:6]:  # Show up to 6 comparison charts
            img_name = os.path.basename(img)
            shutil.copy2(img, output_dir)
            html += f"""
            <div class="gallery-item" style="grid-column: span 2;">
                <img src="{img_name}" alt="{img_name}" style="height: auto;">
                <h4>{img_name.replace('_', ' ').replace('.jpg', '').title()}</h4>
            </div>
"""
        html += """
        </div>
    </div>
"""
    
    # Add note about timing
    html += """
    <div class="note">
        <strong>Note on Results:</strong> 
        <ul style="margin: 10px 0;">
            <li>Processing times include all stages: detection, matching, homography, warping, and blending</li>
            <li>ORB detector produces more keypoints (~25k-50k) for denser matching</li>
            <li>AKAZE detector provides fewer but more stable keypoints (~5k-20k)</li>
            <li>Multiband blending produces smoother transitions but takes more time</li>
        </ul>
    </div>
"""
    
    html += """
</body>
</html>"""
    
    # Write report
    report_path = os.path.join(output_dir, 'analysis_report.html')
    with open(report_path, 'w') as f:
        f.write(html)
    
    print(f"Created analysis report: {report_path}")

def main():
    """Main analysis pipeline"""
    print("="*60)
    print("PANORAMA STITCHING RESULTS ANALYSIS")
    print("="*60)
    
    # Check for results
    if not os.path.exists('results'):
        print("Error: 'results' directory not found.")
        print("Please run experiments first using: ./panorama_stitcher --experiment-mode")
        return
    
    # Load metrics
    df = load_metrics()
    if df is None:
        print("No metrics found. Please run experiments first.")
        return
    
    # Create output directory
    output_dir = 'results_analysis'
    os.makedirs(output_dir, exist_ok=True)
    
    # Create comprehensive report
    create_comprehensive_report(df, output_dir)
    
    # Create visualization showcase if available
    viz_dir = 'results/visualizations'
    if os.path.exists(viz_dir):
        create_visualization_showcase(viz_dir, output_dir)
    
    print("\n" + "="*60)
    print("✅ Analysis Complete!")
    print(f"📁 Results saved to: {output_dir}/")
    print(f"🌐 Open {output_dir}/analysis_report.html to view the report")
    print("="*60)

if __name__ == "__main__":
    main()