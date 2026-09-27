package com.execcommand

import com.facebook.react.bridge.ReactApplicationContext

class ExecCommandModule(reactContext: ReactApplicationContext) :
  NativeExecCommandSpec(reactContext) {

  override fun multiply(a: Double, b: Double): Double {
    return a * b
  }

  companion object {
    const val NAME = NativeExecCommandSpec.NAME
  }
}
