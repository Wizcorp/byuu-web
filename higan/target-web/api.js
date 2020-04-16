const EventEmitter = require('eventemitter3')
const lib = require('./byuu-web-lib.js')

let initialized = false;

module.exports = new EventEmitter();

exports.init = async function (height, width) {
  await lib
  lib.init(height, width)

  // Set callbacks, patch into event emission
  lib.onFrameStart(() => exports.emit('frame.start'))
  lib.onFrame((frameData) => exports.emit('frame', frameData))
  lib.onFrameEnd(() => exports.emit('frame.end'))
}

function getModule() {
  if (!initialized) {
    throw new Error('Emulator module was not initialized before use')
  }

  return lib
}

exports.getEmulatorNameForFilename = (filename) => getModule().getEmulatorNameForFilename(filename)

exports.setEmulator = (emulatorName) => getModule().setEmulator(emulatorName)

exports.setEmulatorForFilename = (filename) => getModule().setEmulatorForFilename(filename)

exports.load = (filename, romData, saveFiles) => getModule().load(filename, romData, saveFiles || {})

exports.loadURL = async (url, saveFiles) => new Promise((resolve, reject) => {
  getModule().loadURL(url, saveFiles || {}, (errorString, info) => {
    if (error) {
      return reject(new Error(errorString))
    }

    resolve(info)
  })
})

exports.unload = () => getModule().unload()

exports.start = () => getModule().start()

exports.run = () => getModule().run()

exports.stop = () => getModule().stop()

exports.isStarted = () => getModule().isStarted()

exports.isRunning = () => getModule().isRunning()

exports.resize = (height, width) => getModule().resize(height, width)

exports.connectPeripheral = (portName, peripheralName) => getModule().connectPeripheral(portName, peripheralName)

exports.disconnectPeripheral = (portName) => getModule().disconnectPeripheral(portName)

exports.setButton = (portName, buttonName, value) => getModule().setButton(portName, buttonName, value)

exports.getROMInfo = (filename, romData) => getModule().getROMInfo(filename, romData)

exports.stateSave = async () => new Promise((resolve) => {
  getModule().stateSave(resolve)
})

exports.stateLoad = (stateData) => getModule().stateLoad(stateData)

// This mangling is required to avoid issues with IndexedDB storage
// (e.g some API seem to access the buffer attribute of views, which in this case
// is the entire heap!)
exports.save = () => {
  const saveFiles = getModule().save()
  for (const [filename, { buffer, byteOffset, byteLength }] of Object.entries(saveFiles)) {
    saveFiles[filename] = buffer.slice(byteOffset, byteOffset + byteLength);
  }

  return saveFiles
}
