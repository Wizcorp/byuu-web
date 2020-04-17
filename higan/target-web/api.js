let lib = {};
let initialized = false;

function getBinaryPath() {
  try {
    return require('./byuu-web-lib.wasm').default
  } catch (error) {
    throw new Error('Failed to load WASM code - see https://github.com/Wizcorp/byuu-web#failed-to-load-wasm-code')
  }
}

function getModule() {
  if (!initialized) {
    throw new Error('Emulator module was not initialized before use')
  }

  return lib
}

const EventEmitter = require('eventemitter3')
const emulator = new EventEmitter()

// String enums
emulator.Emulator = {
  Famicom: 'Famicom',
  SuperFamicom: 'Superfamicom',
}

emulator.EmulatorEvent = {
  FrameStart: 'frame.start',
  FrameEnd: 'frame.end',
}


const { default: Module } = require('./byuu-web-lib.js')

emulator.init = async function (canvas, height, width) {
  if (!canvas) {
    throw new Error('canvas parameter is not defined')
  }

  if (!height) {
    throw new Error('height parameter is not defined')
  }

  if (!width) {
    throw new Error('width parameter is not defined')
  }

  if (initialized) { 
    return
  }

  const domElementWithIDCanvas = document.getElementById('canvas')
  if (domElementWithIDCanvas && !canvas.isSameNode(domElementWithIDCanvas)) {
    throw new Error('The DOM ID attribute "canvas" MUST be assigned to the emulator\'s canvas')
  }

  // Emscripten's SDL port enforces that the id of the canvas be canvas
  // https://github.com/emscripten-ports/SDL2/blob/952f889879ba3d934249fc7d93b992f91f75a5cd/src/video/emscripten/SDL_emscriptenvideo.c#L217
  canvas.id = 'canvas'

  return new Promise((resolve, reject) => {
    // Module isn't a real promise, and unless we set
    // things as follow the code seem to tight-loop
    Module({ 
      locateFile: (filename, prefix) =>  {
        if (filename === 'byuu-web-lib.wasm') {
          return getBinaryPath()
        } 

        return prefix + filename
      },
      canvas
    }).then((l) => {
      lib = l
      lib.init(height, width)

      // Set callbacks, patch into event emission
      lib.onFrameStart(() => emulator.emit('frame.start'))
      lib.onFrameEnd(() => emulator.emit('frame.end'))
      
      initialized = true
      resolve();
    })
  })
}

emulator.getEmulatorNameForFilename = (filename) => getModule().getEmulatorNameForFilename(filename)

emulator.setEmulator = (emulatorName) => getModule().setEmulator(emulatorName)

emulator.setEmulatorForFilename = (filename) => getModule().setEmulatorForFilename(filename)

emulator.load = (filename, romData, saveFiles) => getModule().load(filename, romData, saveFiles || {})

emulator.loadURL = async (url, saveFiles) => new Promise((resolve, reject) => {
  getModule().loadURL(url, saveFiles || {}, (errorString, info) => {
    if (errorString) {
      return reject(new Error(errorString))
    }

    resolve(info)
  })
})

emulator.unload = () => getModule().unload()

emulator.start = () => getModule().start()

emulator.run = () => getModule().run()

emulator.stop = () => getModule().stop()

emulator.isStarted = () => getModule().isStarted()

emulator.isRunning = () => getModule().isRunning()

emulator.resize = (height, width) => getModule().resize(height, width)

emulator.connectPeripheral = (portName, peripheralName) => getModule().connectPeripheral(portName, peripheralName)

emulator.disconnectPeripheral = (portName) => getModule().disconnectPeripheral(portName)

emulator.setButton = (portName, buttonName, value) => getModule().setButton(portName, buttonName, value)

emulator.getROMInfo = (filename, romData) => getModule().getROMInfo(filename, romData)

emulator.stateSave = async () => new Promise((resolve) => {
  getModule().stateSave(resolve)
})

emulator.stateLoad = (stateData) => getModule().stateLoad(stateData)

// This mangling is required to avoid issues with IndexedDB storage
// (e.g some API seem to access the buffer attribute of views, which in this case
// is the entire heap!)
emulator.save = () => {
  const saveFiles = getModule().save()
  for (const [filename, { buffer, byteOffset, byteLength }] of Object.entries(saveFiles)) {
    saveFiles[filename] = buffer.slice(byteOffset, byteOffset + byteLength);
  }

  return saveFiles
}

module.exports = emulator