# vk-spoof-gpu

This Vulkan layer allows you to spoof your GPU's name, ID, and vendor ID.
Useful when games and other applications (erroneously) gate features behind specific GPU models or vendors.
For example, many games won't allow ray tracing on NVIDIA RTX-series workstation GPUs, even though they support it just fine.

## Installation

Build the layer using CMake and install it to your system.

```bash
git clone https://github.com/tom94/vk-spoof-gpu
cd vk-spoof-gpu
cmake -S src -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cmake --install build
```

On NixOS, you can install the layer system-wide as follows:

```nix
# flake.nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    vk-spoof-gpu.url = "github:Tom94/vk-spoof-gpu";
  };

  outputs = { self, nixpkgs, vk-spoof-gpu, ... }: {
    # then use it wherever you list packages:
    # environment.systemPackages = with pkgs; [
    #   vk-spoof-gpu.packages.${system}.default
    # ];
  };
}
```

## Usage

Append `VK_LAYER_spoof_gpu` to the `VK_INSTANCE_LAYERS` environment variable and set `SPOOF_DEVICE_NAME`, `SPOOF_DEVICE_ID`, and `SPOOF_VENDOR_ID` to the desired values.

For example, to spoof an NVIDIA GeForce RTX 5090 in a Steam game, use this:
```bash
VK_INSTANCE_LAYERS=VK_LAYER_spoof_gpu SPOOF_DEVICE_NAME="NVIDIA GeForce RTX 5090" SPOOF_DEVICE_ID=0x2b85 SPOOF_VENDOR_ID=0x10de %command%
```

## License

GPL-3.0 License. See [LICENSE](LICENSE) for more details.
