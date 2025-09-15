# 🚀 QUICK START - Panorama Stitching

## The Only Command You Need:

```bash
./RUN.sh
```

That's it! This will:
1. ✅ Check if you have everything installed
2. ✅ Build the project
3. ✅ Run a quick test
4. ✅ Show you an interactive menu

---

## 🎯 What This Project Does

Takes two overlapping photos → Combines them into one panorama

**Example:**
```
[Photo 1] + [Photo 2] = [Panorama]
   📷          📷           🖼️
```

---

## 📋 First Time Setup (if RUN.sh tells you something is missing)

### Ubuntu/Linux:
```bash
sudo apt-get install cmake g++ libopencv-dev python3-pip
pip3 install pandas matplotlib
```

### macOS:
```bash
brew install cmake opencv python3
pip3 install pandas matplotlib
```

---

## 🎮 Menu Options Explained

When you run `./RUN.sh`, you'll see:

1. **Run ALL experiments** → Tests everything thoroughly (5-10 min)
2. **Run quick demo** → See it working in 30 seconds
3. **Test your own images** → Try with your photos
4. **View results** → See pretty charts and analysis
5. **Show documentation** → Learn how it works
6. **Clean rebuild** → Fix any issues
7. **Exit** → Leave

**Beginners:** Start with option 2 (quick demo)!

---

## 📸 Want to Try Your Own Photos?

Your photos need:
- 📐 30-40% overlap (take them from same spot, rotate camera)
- 🌞 Similar lighting (don't mix day/night)
- 📏 Same height (don't tilt up/down)

Then choose option 3 in the menu!

---

## 🆘 Something Not Working?

1. **"cmake not found"** → Install dependencies (see First Time Setup)
2. **"Build failed"** → Try option 6 (clean rebuild)
3. **"No panorama created"** → Your images might not have enough overlap
4. **Still stuck?** → Check README.md for detailed help

---

## 📊 Understanding Results

After running experiments, you get:
- **Panoramas** → The combined images (in `results/`)
- **Report** → Beautiful analysis with graphs (in `results_analysis/`)
- **Metrics** → How well it worked (success rate, matches found, etc.)

---

## 🎓 Learn More

- **README.md** → Detailed documentation
- **PROJECT_STRUCTURE.md** → How the code is organized
- **src/** → The actual C++ code (if you're curious!)

---

## 💡 Pro Tips

- **Indoor photos:** Work best with AKAZE detector
- **Outdoor photos:** Work best with ORB detector
- **Best quality:** Use multiband blending
- **Fastest:** Use simple blending

---

Remember: Just run `./RUN.sh` and follow the menu! 🎉