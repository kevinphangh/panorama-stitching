#!/usr/bin/env python3
"""
Unified analysis and organization script for experiment results
This combines all analysis and organization into one simple script
"""

import os
import shutil
from pathlib import Path

def create_organized_structure():
    """Create the organized folder structure"""
    base = Path("results_organized")
    
    # Create directory structure
    folders = [
        "01_detector_comparison/indoor_scene",
        "01_detector_comparison/outdoor_scene1", 
        "01_detector_comparison/outdoor_scene2",
        "02_ransac_analysis/indoor_scene",
        "02_ransac_analysis/outdoor_scene1",
        "02_ransac_analysis/outdoor_scene2",
        "03_blending_comparison/indoor_scene",
        "03_blending_comparison/outdoor_scene1",
        "03_blending_comparison/outdoor_scene2",
        "04_multi_image_stitching"
    ]
    
    for folder in folders:
        (base / folder).mkdir(parents=True, exist_ok=True)
    
    return base

def organize_files():
    """Organize experiment results into clean structure"""
    base = Path("results_organized")
    
    # Count files organized
    organized = 0
    
    # 1. Detector comparison files
    scenes = ["indoor_scene", "outdoor_scene1", "outdoor_scene2"]
    
    for scene in scenes:
        for detector in ["orb", "akaze"]:
            for pair in ["1_2", "2_3", "1_3"]:
                src = Path(f"results/{scene}_pair_{pair}_{detector}.jpg")
                dst = base / "01_detector_comparison" / scene / f"{detector}_pair_{pair.replace('_', '-')}.jpg"
                if src.exists():
                    shutil.copy2(src, dst)
                    organized += 1
    
    # 2. RANSAC analysis files
    for scene in scenes:
        for threshold in ["1.0", "2.0", "3.0", "4.0", "5.0"]:
            src = Path(f"results/{scene}_ransac_{threshold}.jpg")
            dst = base / "02_ransac_analysis" / scene / f"threshold_{threshold}.jpg"
            if src.exists():
                shutil.copy2(src, dst)
                organized += 1
    
    # 3. Blending comparison files
    for scene in scenes:
        for blend in ["simple", "feather", "multiband"]:
            src = Path(f"results/{scene}_blend_{blend}.jpg")
            dst = base / "03_blending_comparison" / scene / f"{blend}.jpg"
            if src.exists():
                shutil.copy2(src, dst)
                organized += 1
    
    # 4. Multi-image stitching files
    for scene in scenes:
        for detector in ["orb", "akaze"]:
            src = Path(f"results/{scene}_multi_{detector}.jpg")
            scene_short = scene.replace("_scene", "").replace("outdoor", "out")
            dst = base / "04_multi_image_stitching" / f"{scene_short}_{detector}.jpg"
            if src.exists():
                shutil.copy2(src, dst)
                organized += 1
    
    return organized

def create_html_viewers():
    """Create HTML pages for easy viewing"""
    base = Path("results_organized")
    
    # Create main index.html
    html = """<!DOCTYPE html>
<html>
<head>
    <title>Experiment Results - Visual Computing Assignment 1</title>
    <style>
        body {
            font-family: 'Segoe UI', Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 10px;
            padding: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 30px;
        }
        .experiment-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-top: 30px;
        }
        .experiment-card {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 8px;
            text-align: center;
            transition: transform 0.3s;
        }
        .experiment-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        .experiment-card h3 {
            color: #667eea;
            margin-bottom: 15px;
        }
        .scene-link {
            display: block;
            padding: 8px;
            margin: 5px 0;
            background: white;
            border-radius: 5px;
            text-decoration: none;
            color: #333;
            transition: background 0.3s;
        }
        .scene-link:hover {
            background: #e9ecef;
        }
        .stats {
            display: flex;
            justify-content: space-around;
            margin: 30px 0;
            padding: 20px;
            background: #f8f9fa;
            border-radius: 8px;
        }
        .stat {
            text-align: center;
        }
        .stat-number {
            font-size: 2em;
            font-weight: bold;
            color: #667eea;
        }
        .stat-label {
            color: #666;
            margin-top: 5px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔬 Visual Computing Assignment 1 - Results</h1>
        
        <div class="stats">
            <div class="stat">
                <div class="stat-number">48</div>
                <div class="stat-label">Experiments</div>
            </div>
            <div class="stat">
                <div class="stat-number">3</div>
                <div class="stat-label">Scenes</div>
            </div>
            <div class="stat">
                <div class="stat-number">2</div>
                <div class="stat-label">Detectors</div>
            </div>
        </div>
        
        <div class="experiment-grid">
            <div class="experiment-card">
                <h3>🔍 Detector Comparison</h3>
                <a href="01_detector_comparison/indoor_scene/" class="scene-link">Indoor Scene</a>
                <a href="01_detector_comparison/outdoor_scene1/" class="scene-link">Outdoor Scene 1</a>
                <a href="01_detector_comparison/outdoor_scene2/" class="scene-link">Outdoor Scene 2</a>
            </div>
            
            <div class="experiment-card">
                <h3>📊 RANSAC Analysis</h3>
                <a href="02_ransac_analysis/indoor_scene/" class="scene-link">Indoor Scene</a>
                <a href="02_ransac_analysis/outdoor_scene1/" class="scene-link">Outdoor Scene 1</a>
                <a href="02_ransac_analysis/outdoor_scene2/" class="scene-link">Outdoor Scene 2</a>
            </div>
            
            <div class="experiment-card">
                <h3>🎨 Blending Modes</h3>
                <a href="03_blending_comparison/indoor_scene/" class="scene-link">Indoor Scene</a>
                <a href="03_blending_comparison/outdoor_scene1/" class="scene-link">Outdoor Scene 1</a>
                <a href="03_blending_comparison/outdoor_scene2/" class="scene-link">Outdoor Scene 2</a>
            </div>
            
            <div class="experiment-card">
                <h3>🖼️ Multi-Image</h3>
                <a href="04_multi_image_stitching/" class="scene-link">All Scenes</a>
            </div>
        </div>
    </div>
</body>
</html>"""
    
    with open(base / "index.html", "w") as f:
        f.write(html)
    
    # Create simple gallery pages for each experiment type
    # This creates a basic image gallery for each folder
    for folder in base.rglob("*"):
        if folder.is_dir() and not folder.name.startswith("."):
            images = list(folder.glob("*.jpg")) + list(folder.glob("*.png"))
            if images:
                gallery_html = f"""<!DOCTYPE html>
<html>
<head>
    <title>{folder.name.replace('_', ' ').title()}</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        h1 {{ color: #333; }}
        .gallery {{ display: grid; grid-template-columns: repeat(auto-fit, minmax(400px, 1fr)); gap: 20px; }}
        .image-container {{ background: #f5f5f5; padding: 10px; border-radius: 5px; }}
        .image-container img {{ width: 100%; height: auto; display: block; }}
        .image-container h3 {{ margin: 10px 0 5px 0; color: #555; }}
        a {{ text-decoration: none; color: #667eea; }}
        .back {{ margin-bottom: 20px; }}
    </style>
</head>
<body>
    <a href="../" class="back">← Back</a>
    <h1>{folder.name.replace('_', ' ').title()}</h1>
    <div class="gallery">
"""
                for img in sorted(images):
                    name = img.stem.replace('_', ' ').title()
                    gallery_html += f"""
        <div class="image-container">
            <h3>{name}</h3>
            <img src="{img.name}" alt="{name}">
        </div>
"""
                
                gallery_html += """
    </div>
</body>
</html>"""
                
                with open(folder / "index.html", "w") as f:
                    f.write(gallery_html)

def generate_summary():
    """Generate a summary of the results"""
    results_path = Path("results")
    organized_path = Path("results_organized")
    
    # Count files
    total_results = len(list(results_path.glob("*.jpg")))
    organized_files = len(list(organized_path.rglob("*.jpg")))
    
    print("\n" + "="*60)
    print("ANALYSIS & ORGANIZATION COMPLETE")
    print("="*60)
    print(f"\n📊 Results Summary:")
    print(f"  • Total panoramas generated: {total_results}")
    print(f"  • Files organized: {organized_files}")
    print(f"  • HTML viewers created: {len(list(organized_path.rglob('*.html')))}")
    
    print(f"\n📁 Organized Structure:")
    print(f"  • 01_detector_comparison/   - ORB vs AKAZE results")
    print(f"  • 02_ransac_analysis/       - Threshold experiments")
    print(f"  • 03_blending_comparison/   - Blending modes")
    print(f"  • 04_multi_image_stitching/ - Full panoramas")
    
    print(f"\n🌐 View Results:")
    print(f"  Open: results_organized/index.html")
    print("="*60)

def main():
    """Main function"""
    try:
        # Create organized structure
        base = create_organized_structure()
        
        # Organize files
        num_organized = organize_files()
        
        # Create HTML viewers
        create_html_viewers()
        
        # Generate summary
        generate_summary()
        
        # Also run quantitative analysis if available
        print("\nGenerating quantitative analysis...")
        try:
            import subprocess
            subprocess.run(["python3", "scripts/generate_quantitative_analysis.py"], check=True)
            print("  ✓ Quantitative analysis generated")
        except Exception as e:
            print(f"  ! Could not generate quantitative analysis: {e}")
        
        return 0
    except Exception as e:
        print(f"Error during analysis: {e}")
        return 1

if __name__ == "__main__":
    exit(main())