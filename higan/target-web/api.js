// String enums
export const Emulator = {
  Famicom: 'Famicom',
  SuperFamicom: 'Superfamicom',
}

export const EmulatorEvent = {
  FrameStart: 'frame.start',
  FrameEnd: 'frame.end',
}

let lib;
let initialized = false

import Module from './byuu-web-lib.js'

// The following are injected at build time
export const version = '$version'
export const commit = '$commit'
export const dirty = $dirty

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

import EventEmitter from 'eventemitter3'
const byuu = new EventEmitter()
const canvas = document.createElement('canvas')
canvas.id = 'canvas'

byuu.initialize = async function (container, height, width) {
  if (!container) {
    throw new Error('container parameter is not defined')
  }

  if (!height) {
    throw new Error('height parameter is not defined')
  }

  if (!width) {
    throw new Error('width parameter is not defined')
  }

  container.appendChild(canvas)

  if (initialized) {
    lib.resize(height, width)
    return
  }

  // Emscripten's SDL port enforces that the id of the canvas be canvas
  // https://github.com/emscripten-ports/SDL2/blob/952f889879ba3d934249fc7d93b992f91f75a5cd/src/video/emscripten/SDL_emscriptenvideo.c#L217
  const domElementWithIDCanvas = document.getElementById('canvas')
  if (domElementWithIDCanvas && !canvas.isSameNode(domElementWithIDCanvas)) {
    throw new Error('The DOM ID attribute "canvas" is reserved by byuu for it\'s own canvas')
  }

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
    }).then((result) => {
      lib = result
      lib.initialize(document.title || 'byuu', height, width)
  
      // Set callbacks, patch into event emission
      lib.onFrameStart(() => byuu.emit('frame.start'))
      lib.onFrameEnd(() => byuu.emit('frame.end'))
      
      initialized = true
      resolve()
    })
  })
}

byuu.terminate = () => {
  byuu.stop()
  byuu.unload()
  getModule().terminate()
  canvas.parentElement.removeChild(canvas)
}

byuu.getEmulatorForFilename = (filename) => getModule().getEmulatorForFilename(filename)

byuu.setEmulator = (emulator) => getModule().setEmulator(emulator)

byuu.setEmulatorForFilename = (filename) => getModule().setEmulatorForFilename(filename)

byuu.load = (romData, saveFiles) => getModule().load(romData, saveFiles || {})

byuu.loadURL = async (url, saveFiles) => new Promise((resolve, reject) => {
  getModule().loadURL(url, saveFiles || {}, (errorString, info) => {
    if (errorString) {
      return reject(new Error(errorString))
    }

    resolve(info)
  })
})

byuu.unload = () => getModule().unload()

byuu.start = () => getModule().start()

byuu.run = () => getModule().run()

byuu.stop = () => getModule().stop()

byuu.isStarted = () => getModule().isStarted()

byuu.isRunning = () => getModule().isRunning()

byuu.resize = (height, width) => getModule().resize(height, width)

byuu.setVolume = (volume) => getModule().setVolume(volume)

byuu.setMute = (mute) => getModule().setMute(mute)

byuu.connectPeripheral = (portName, peripheralName) => getModule().connectPeripheral(portName, peripheralName)

byuu.disconnectPeripheral = (portName) => getModule().disconnectPeripheral(portName)

byuu.setButton = (portName, buttonName, value) => getModule().setButton(portName, buttonName, value)

byuu.getROMInfo = (filename, romData) => getModule().getROMInfo(filename, romData)

byuu.stateSave = async () => new Promise((resolve) => {
  getModule().stateSave(({ buffer, byteOffset, byteLength }) => {
    const array = new Uint8Array(buffer.slice(byteOffset, byteOffset + byteLength))
    resolve(array)
  })
})

byuu.stateLoad = (stateData) => getModule().stateLoad(stateData)

// This mangling is required to avoid issues with IndexedDB storage
// (e.g some API seem to access the buffer attribute of views, which in this case
// is the entire heap!)
byuu.save = () => {
  const saveFiles = getModule().save()
  for (const [filename, { buffer, byteOffset, byteLength }] of Object.entries(saveFiles)) {
    saveFiles[filename] = new Uint8Array(buffer.slice(byteOffset, byteOffset + byteLength))
  }

  return saveFiles
}

export default byuu
