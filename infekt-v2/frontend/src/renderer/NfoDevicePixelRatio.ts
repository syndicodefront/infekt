const NfoDevicePixelRatio = () => {
  const MIN = 1;
  const MAX = 4;

  let devicePixelRatio = window.devicePixelRatio || 1;

  devicePixelRatio = devicePixelRatio < MIN ? MIN : Math.round(devicePixelRatio);
  devicePixelRatio = devicePixelRatio > MAX ? MAX : devicePixelRatio;

  return devicePixelRatio;
};

export default NfoDevicePixelRatio;
