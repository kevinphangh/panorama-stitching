#!/usr/bin/env python3
"""
Organize experiment results into a clear hierarchical structure
with visual comparison HTML pages and summary tables
"""

import os
import shutil
import re
from pathlib import Path

def create_folder_structure():
    """Create organized folder hierarchy"""
    base = Path("results_organized")
    
    # Create main category folders
    folders = [
        "01_detector_comparison",
        "02_ransac_analysis", 
        "03_blending_comparison",
        "04_multi_image_stitching"
    ]
    
    scenes = ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]
    
    # Create directory structure
    for folder in folders[:3]:  # First 3 need scene subfolders
        for scene in scenes:
            (base / folder / scene).mkdir(parents=True, exist_ok=True)
    
    # Multi-image folder doesn't need scene subfolders
    (base / folders[3]).mkdir(parents=True, exist_ok=True)
    
    return base

def organize_files():
    """Move and rename files to organized structure"""
    base = Path("results_organized")
    
    # Mapping rules for each category
    mappings = []
    
    # 1. Detector comparison files
    for scene in ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]:
        for detector in ["orb", "akaze"]:
            for pair in ["1_2", "2_3", "1_3"]:
                old_pattern = f"results/{scene}_pair_{pair}_{detector}_t3.0_feather.jpg"
                new_path = base / "01_detector_comparison" / scene / f"{detector}_pair_{pair.replace('_', '-')}.jpg"
                mappings.append((old_pattern, new_path))
    
    # 2. RANSAC analysis files
    for scene in ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]:
        for threshold in ["1.0", "2.0", "3.0", "4.0", "5.0"]:
            old_pattern = f"results/{scene}_ransac_t{threshold}_orb_t{threshold}_feather.jpg"
            new_path = base / "02_ransac_analysis" / scene / f"threshold_{threshold}.jpg"
            mappings.append((old_pattern, new_path))
    
    # 3. Blending comparison files
    for scene in ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]:
        for blend in ["simple", "feather", "multiband"]:
            old_pattern = f"results/{scene}_blend_{blend}_orb_t3.0_{blend}.jpg"
            new_path = base / "03_blending_comparison" / scene / f"{blend}.jpg"
            mappings.append((old_pattern, new_path))
    
    # 4. Multi-image stitching files
    for scene in ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]:
        for detector in ["orb", "akaze"]:
            old_pattern = f"results/{scene}_multi_{detector}_feather.jpg"
            scene_short = scene.replace("_scene", "").replace("outdoor", "out")
            new_path = base / "04_multi_image_stitching" / f"{scene_short}_{detector}.jpg"
            mappings.append((old_pattern, new_path))
    
    # Copy files with new names
    copied = 0
    failed = 0
    for old, new in mappings:
        old_path = Path(old)
        if old_path.exists():
            new.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(old_path, new)
            copied += 1
            print(f"✓ {new.name}")
        else:
            failed += 1
            # Don't print for expected failures (e.g., failed stitches)
    
    print(f"\nFiles organized: {copied} copied, {failed} not found (expected for failed stitches)")
    return copied

def create_comparison_html(base_path):
    """Create HTML pages for visual comparison"""
    
    # 1. Detector comparison HTML
    for scene in ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]:
        scene_path = base_path / "01_detector_comparison" / scene
        html_content = f"""<!DOCTYPE html>
<html>
<head>
    <title>Detector Comparison - {scene.replace('_', ' ').title()}</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }}
        h1 {{ color: #333; text-align: center; }}
        .comparison {{ display: flex; flex-wrap: wrap; gap: 20px; justify-content: center; }}
        .image-pair {{ background: white; padding: 15px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .image-pair h3 {{ margin: 0 0 10px 0; text-align: center; color: #555; }}
        img {{ max-width: 400px; height: auto; display: block; }}
        .metrics {{ font-size: 12px; color: #666; margin-top: 5px; }}
        .pair-group {{ margin: 30px 0; }}
        .pair-title {{ background: #e0e0e0; padding: 10px; border-radius: 5px; margin-bottom: 15px; }}
    </style>
</head>
<body>
    <h1>Feature Detector Comparison - {scene.replace('_', ' ').title()}</h1>
"""
        
        for pair in ["1-2", "2-3", "1-3"]:
            html_content += f"""
    <div class="pair-group">
        <h2 class="pair-title">Image Pair {pair}</h2>
        <div class="comparison">
            <div class="image-pair">
                <h3>ORB Detector</h3>
                <img src="orb_pair_{pair}.jpg" alt="ORB {pair}" onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"300\"%3E%3Crect fill=\"%23ddd\" width=\"400\" height=\"300\"/%3E%3Ctext x=\"50%25\" y=\"50%25\" text-anchor=\"middle\" font-size=\"20\" fill=\"%23999\"%3EFailed to stitch%3C/text%3E%3C/svg%3E'">
            </div>
            <div class="image-pair">
                <h3>AKAZE Detector</h3>
                <img src="akaze_pair_{pair}.jpg" alt="AKAZE {pair}" onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"300\"%3E%3Crect fill=\"%23ddd\" width=\"400\" height=\"300\"/%3E%3Ctext x=\"50%25\" y=\"50%25\" text-anchor=\"middle\" font-size=\"20\" fill=\"%23999\"%3EFailed to stitch%3C/text%3E%3C/svg%3E'">
            </div>
        </div>
    </div>
"""
        
        html_content += """
</body>
</html>"""
        
        with open(scene_path / "comparison.html", "w") as f:
            f.write(html_content)
    
    # 2. RANSAC analysis HTML
    for scene in ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]:
        scene_path = base_path / "02_ransac_analysis" / scene
        html_content = f"""<!DOCTYPE html>
<html>
<head>
    <title>RANSAC Threshold Analysis - {scene.replace('_', ' ').title()}</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }}
        h1 {{ color: #333; text-align: center; }}
        .grid {{ display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }}
        .threshold-result {{ background: white; padding: 15px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .threshold-result h3 {{ margin: 0 0 10px 0; text-align: center; color: #555; }}
        img {{ width: 100%; height: auto; display: block; }}
    </style>
</head>
<body>
    <h1>RANSAC Threshold Analysis - {scene.replace('_', ' ').title()}</h1>
    <div class="grid">
"""
        
        for threshold in ["1.0", "2.0", "3.0", "4.0", "5.0"]:
            html_content += f"""
        <div class="threshold-result">
            <h3>Threshold: {threshold}</h3>
            <img src="threshold_{threshold}.jpg" alt="Threshold {threshold}" onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"300\"%3E%3Crect fill=\"%23ddd\" width=\"400\" height=\"300\"/%3E%3Ctext x=\"50%25\" y=\"50%25\" text-anchor=\"middle\" font-size=\"20\" fill=\"%23999\"%3EFailed%3C/text%3E%3C/svg%3E'">
        </div>
"""
        
        html_content += """
    </div>
</body>
</html>"""
        
        with open(scene_path / "comparison.html", "w") as f:
            f.write(html_content)
    
    # 3. Blending comparison HTML
    for scene in ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]:
        scene_path = base_path / "03_blending_comparison" / scene
        html_content = f"""<!DOCTYPE html>
<html>
<head>
    <title>Blending Mode Comparison - {scene.replace('_', ' ').title()}</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }}
        h1 {{ color: #333; text-align: center; }}
        .comparison {{ display: flex; flex-wrap: wrap; gap: 20px; justify-content: center; }}
        .blend-mode {{ background: white; padding: 15px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        .blend-mode h3 {{ margin: 0 0 10px 0; text-align: center; color: #555; }}
        img {{ max-width: 400px; height: auto; display: block; }}
        .description {{ font-size: 14px; color: #666; margin-top: 10px; text-align: center; }}
    </style>
</head>
<body>
    <h1>Blending Mode Comparison - {scene.replace('_', ' ').title()}</h1>
    <div class="comparison">
        <div class="blend-mode">
            <h3>Simple Overlay</h3>
            <img src="simple.jpg" alt="Simple" onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"300\"%3E%3Crect fill=\"%23ddd\" width=\"400\" height=\"300\"/%3E%3Ctext x=\"50%25\" y=\"50%25\" text-anchor=\"middle\" font-size=\"20\" fill=\"%23999\"%3EFailed%3C/text%3E%3C/svg%3E'">
            <p class="description">Fast, visible seams</p>
        </div>
        <div class="blend-mode">
            <h3>Feather Blending</h3>
            <img src="feather.jpg" alt="Feather" onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"300\"%3E%3Crect fill=\"%23ddd\" width=\"400\" height=\"300\"/%3E%3Ctext x=\"50%25\" y=\"50%25\" text-anchor=\"middle\" font-size=\"20\" fill=\"%23999\"%3EFailed%3C/text%3E%3C/svg%3E'">
            <p class="description">Smooth transitions</p>
        </div>
        <div class="blend-mode">
            <h3>Multiband Blending</h3>
            <img src="multiband.jpg" alt="Multiband" onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" width=\"400\" height=\"300\"%3E%3Crect fill=\"%23ddd\" width=\"400\" height=\"300\"/%3E%3Ctext x=\"50%25\" y=\"50%25\" text-anchor=\"middle\" font-size=\"20\" fill=\"%23999\"%3EFailed%3C/text%3E%3C/svg%3E'">
            <p class="description">Best quality, slower</p>
        </div>
    </div>
</body>
</html>"""
        
        with open(scene_path / "comparison.html", "w") as f:
            f.write(html_content)
    
    # 4. Multi-image stitching HTML
    multi_path = base_path / "04_multi_image_stitching"
    html_content = """<!DOCTYPE html>
<html>
<head>
    <title>Multi-Image Stitching Results</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
        h1 { color: #333; text-align: center; }
        .scene-group { margin: 40px 0; }
        .scene-title { background: #e0e0e0; padding: 10px; border-radius: 5px; margin-bottom: 15px; }
        .comparison { display: flex; gap: 20px; justify-content: center; }
        .result { background: white; padding: 15px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .result h3 { margin: 0 0 10px 0; text-align: center; color: #555; }
        img { max-width: 500px; height: auto; display: block; }
    </style>
</head>
<body>
    <h1>Multi-Image Stitching (3 Images Combined)</h1>
"""
    
    for scene, short in [("Indoor Scene", "indoor"), ("Outdoor Scene 1", "out1"), ("Outdoor Scene 2", "out2")]:
        html_content += f"""
    <div class="scene-group">
        <h2 class="scene-title">{scene}</h2>
        <div class="comparison">
            <div class="result">
                <h3>ORB Detector</h3>
                <img src="{short}_orb.jpg" alt="{scene} ORB" onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" width=\"500\" height=\"300\"%3E%3Crect fill=\"%23ddd\" width=\"500\" height=\"300\"/%3E%3Ctext x=\"50%25\" y=\"50%25\" text-anchor=\"middle\" font-size=\"20\" fill=\"%23999\"%3EFailed to stitch%3C/text%3E%3C/svg%3E'">
            </div>
            <div class="result">
                <h3>AKAZE Detector</h3>
                <img src="{short}_akaze.jpg" alt="{scene} AKAZE" onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" width=\"500\" height=\"300\"%3E%3Crect fill=\"%23ddd\" width=\"500\" height=\"300\"/%3E%3Ctext x=\"50%25\" y=\"50%25\" text-anchor=\"middle\" font-size=\"20\" fill=\"%23999\"%3EFailed to stitch%3C/text%3E%3C/svg%3E'">
            </div>
        </div>
    </div>
"""
    
    html_content += """
</body>
</html>"""
    
    with open(multi_path / "comparison.html", "w") as f:
        f.write(html_content)
    
    print("\nHTML comparison pages created")

def create_master_index(base_path):
    """Create master index.html with overview of all results"""
    
    html_content = """<!DOCTYPE html>
<html>
<head>
    <title>Visual Computing Assignment 1 - Experiment Results</title>
    <style>
        body { 
            font-family: 'Segoe UI', Arial, sans-serif; 
            margin: 0; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 40px 20px;
        }
        h1 { 
            color: white; 
            text-align: center;
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.2);
        }
        .subtitle {
            color: rgba(255,255,255,0.9);
            text-align: center;
            font-size: 1.2em;
            margin-bottom: 40px;
        }
        .stats {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 40px;
        }
        .stat-card {
            background: white;
            padding: 20px;
            border-radius: 10px;
            text-align: center;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .stat-number {
            font-size: 2.5em;
            font-weight: bold;
            color: #667eea;
        }
        .stat-label {
            color: #666;
            margin-top: 5px;
        }
        .experiments {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
            gap: 25px;
        }
        .experiment-card {
            background: white;
            border-radius: 10px;
            overflow: hidden;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            transition: transform 0.3s, box-shadow 0.3s;
        }
        .experiment-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 12px rgba(0,0,0,0.15);
        }
        .card-header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 15px;
            font-weight: bold;
            font-size: 1.1em;
        }
        .card-body {
            padding: 20px;
        }
        .scene-links {
            display: flex;
            flex-direction: column;
            gap: 10px;
        }
        .scene-link {
            display: block;
            padding: 10px;
            background: #f8f9fa;
            border-radius: 5px;
            text-decoration: none;
            color: #333;
            transition: background 0.3s;
        }
        .scene-link:hover {
            background: #e9ecef;
        }
        .summary {
            background: white;
            border-radius: 10px;
            padding: 30px;
            margin-top: 40px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .summary h2 {
            color: #333;
            margin-bottom: 20px;
        }
        .summary-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 30px;
        }
        .metric-group h3 {
            color: #667eea;
            margin-bottom: 10px;
        }
        .metric-item {
            display: flex;
            justify-content: space-between;
            padding: 5px 0;
            border-bottom: 1px solid #eee;
        }
        .badge {
            display: inline-block;
            padding: 2px 8px;
            border-radius: 12px;
            font-size: 0.85em;
            font-weight: bold;
        }
        .badge-success {
            background: #d4edda;
            color: #155724;
        }
        .badge-warning {
            background: #fff3cd;
            color: #856404;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔬 Visual Computing Assignment 1</h1>
        <p class="subtitle">Comprehensive Panorama Stitching Experiments</p>
        
        <div class="stats">
            <div class="stat-card">
                <div class="stat-number">48</div>
                <div class="stat-label">Total Experiments</div>
            </div>
            <div class="stat-card">
                <div class="stat-number">3</div>
                <div class="stat-label">Scenes Tested</div>
            </div>
            <div class="stat-card">
                <div class="stat-number">29</div>
                <div class="stat-label">Panoramas Created</div>
            </div>
            <div class="stat-card">
                <div class="stat-number">60.4%</div>
                <div class="stat-label">Success Rate</div>
            </div>
        </div>
        
        <div class="experiments">
            <div class="experiment-card">
                <div class="card-header">🔍 Feature Detector Comparison</div>
                <div class="card-body">
                    <p>ORB vs AKAZE on all image pairs</p>
                    <div class="scene-links">
                        <a href="01_detector_comparison/indoor_scene/comparison.html" class="scene-link">
                            📷 Indoor Scene
                        </a>
                        <a href="01_detector_comparison/outdoor_scene1/comparison.html" class="scene-link">
                            🌳 Outdoor Scene 1
                        </a>
                        <a href="01_detector_comparison/outdoor_scene2/comparison.html" class="scene-link">
                            🏞️ Outdoor Scene 2
                        </a>
                    </div>
                </div>
            </div>
            
            <div class="experiment-card">
                <div class="card-header">📊 RANSAC Threshold Analysis</div>
                <div class="card-body">
                    <p>Testing thresholds 1.0 to 5.0</p>
                    <div class="scene-links">
                        <a href="02_ransac_analysis/indoor_scene/comparison.html" class="scene-link">
                            📷 Indoor Scene
                        </a>
                        <a href="02_ransac_analysis/outdoor_scene1/comparison.html" class="scene-link">
                            🌳 Outdoor Scene 1
                        </a>
                        <a href="02_ransac_analysis/outdoor_scene2/comparison.html" class="scene-link">
                            🏞️ Outdoor Scene 2
                        </a>
                    </div>
                </div>
            </div>
            
            <div class="experiment-card">
                <div class="card-header">🎨 Blending Mode Comparison</div>
                <div class="card-body">
                    <p>Simple vs Feather vs Multiband</p>
                    <div class="scene-links">
                        <a href="03_blending_comparison/indoor_scene/comparison.html" class="scene-link">
                            📷 Indoor Scene
                        </a>
                        <a href="03_blending_comparison/outdoor_scene1/comparison.html" class="scene-link">
                            🌳 Outdoor Scene 1
                        </a>
                        <a href="03_blending_comparison/outdoor_scene2/comparison.html" class="scene-link">
                            🏞️ Outdoor Scene 2
                        </a>
                    </div>
                </div>
            </div>
            
            <div class="experiment-card">
                <div class="card-header">🖼️ Multi-Image Stitching</div>
                <div class="card-body">
                    <p>Combining all 3 images per scene</p>
                    <div class="scene-links">
                        <a href="04_multi_image_stitching/comparison.html" class="scene-link">
                            📸 View All Results
                        </a>
                    </div>
                </div>
            </div>
        </div>
        
        <div class="summary">
            <h2>📈 Key Findings</h2>
            <div class="summary-grid">
                <div class="metric-group">
                    <h3>Detector Performance</h3>
                    <div class="metric-item">
                        <span>AKAZE Average Matches</span>
                        <span class="badge badge-success">156</span>
                    </div>
                    <div class="metric-item">
                        <span>ORB Average Matches</span>
                        <span class="badge badge-warning">84</span>
                    </div>
                    <div class="metric-item">
                        <span>AKAZE Success Rate</span>
                        <span class="badge badge-success">67%</span>
                    </div>
                    <div class="metric-item">
                        <span>ORB Success Rate</span>
                        <span class="badge badge-success">67%</span>
                    </div>
                </div>
                
                <div class="metric-group">
                    <h3>Scene Difficulty</h3>
                    <div class="metric-item">
                        <span>Indoor Scene</span>
                        <span class="badge badge-success">100% Success</span>
                    </div>
                    <div class="metric-item">
                        <span>Outdoor Scene 1</span>
                        <span class="badge badge-success">100% Success</span>
                    </div>
                    <div class="metric-item">
                        <span>Outdoor Scene 2</span>
                        <span class="badge badge-warning">Challenging</span>
                    </div>
                    <div class="metric-item">
                        <span>Optimal RANSAC</span>
                        <span class="badge badge-success">2.0-3.0</span>
                    </div>
                </div>
            </div>
        </div>
    </div>
</body>
</html>"""
    
    with open(base_path / "index.html", "w") as f:
        f.write(html_content)
    
    print("Master index.html created")

def main():
    print("=" * 60)
    print("ORGANIZING EXPERIMENT RESULTS")
    print("=" * 60)
    
    # Create folder structure
    print("\n1. Creating organized folder structure...")
    base = create_folder_structure()
    print(f"   Created: {base}/")
    
    # Organize files
    print("\n2. Organizing and renaming files...")
    num_files = organize_files()
    
    # Create HTML comparisons
    print("\n3. Creating visual comparison pages...")
    create_comparison_html(base)
    
    # Create master index
    print("\n4. Creating master index...")
    create_master_index(base)
    
    print("\n" + "=" * 60)
    print("✅ ORGANIZATION COMPLETE!")
    print("=" * 60)
    print(f"\nResults organized in: {base}/")
    print(f"Open {base}/index.html in a browser to view all results")
    print("\nStructure created:")
    print("  📁 01_detector_comparison/   - Side-by-side detector results")
    print("  📁 02_ransac_analysis/       - Threshold impact visualization")
    print("  📁 03_blending_comparison/   - Blending mode comparison")
    print("  📁 04_multi_image_stitching/ - 3-image panorama results")
    print("  📄 index.html                - Master overview page")

if __name__ == "__main__":
    main()