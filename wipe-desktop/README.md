# wipe-desktop

## Project Structure
```
wipe-desktop/
├── include/              # Header files
├── src/                  # Source files
├── Makefile              # Build configuration
├── README.md             # Basic documentation for wipe-desktop
```


## Build & Run
### Install Prerequisites
```bash
sudo apt update
sudo apt install -y pkg-config make gcc build-essential libgtk-4-dev libudev-dev
```

### Build
```bash
make
```

### Run
```bash
LD_LIBRARY_PATH=build/libs ./build/wipe
```

### Clean
```bash
make clean
```