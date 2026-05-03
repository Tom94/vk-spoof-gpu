{
  lib,
  stdenv,
  cmake,
  vulkan-headers,
  vulkan-loader,
  pkg-config,
  writeText,
}:

stdenv.mkDerivation {
  pname = "vulkan-spoof-layer";
  version = "0.1.0";

  src = ./src;

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    vulkan-headers
    vulkan-loader
  ];

  postInstall = ''
    substituteInPlace $out/share/vulkan/explicit_layer.d/VkLayer_spoof_gpu.json \
      --replace-fail "../lib/" "$out/lib/"
  '';

  setupHook = writeText "setup-hook" ''
    addToSearchPath XDG_DATA_DIRS @out@/share
  '';

  meta = {
    description = "Vulkan layer to spoof GPU vendor/device ID and name via environment variables";
    homepage = "https://github.com/volca02/spoof_vendorid";
    license = lib.licenses.asl20;
    platforms = lib.platforms.linux;
  };
}
