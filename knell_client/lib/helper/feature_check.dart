import 'package:flutter/foundation.dart' show kIsWeb;
import 'dart:io' show Platform;

bool get canHover {
  if (kIsWeb) return true;
  try {
    return Platform.isWindows || Platform.isLinux || Platform.isMacOS;
  } catch (_) {
    return false;
  }
}
