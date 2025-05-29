import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'screens/notification_list.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'model/knell_notification.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Hive.initFlutter();
  Hive.registerAdapter(KnellNotificationAdapter());
  await Hive.openBox<KnellNotification>('notifications');

  final box = Hive.box<KnellNotification>('notifications');

  if (kDebugMode && box.isEmpty) {
    // Only seed if in debug mode and the box is empty!
    box.addAll([
      KnellNotification(
        title: 'Weather Alert',
        message: 'Rain expected tomorrow',
        time: '10:30 AM',
      ),
      KnellNotification(
        title: 'Reminder',
        message: 'Meeting at 2 PM',
        time: '9:15 AM',
      ),
    ]);
  }

  runApp(KnellClient());
}

class KnellClient extends StatefulWidget {
  const KnellClient({super.key});

  @override
  KnellClientState createState() => KnellClientState();
}

class KnellClientState extends State<KnellClient> {
  ThemeMode _themeMode = ThemeMode.light;

  void _toggleTheme() {
    setState(() {
      _themeMode = _themeMode == ThemeMode.light
          ? ThemeMode.dark
          : ThemeMode.light;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Environmental Notifications',
      debugShowCheckedModeBanner: true,
      theme: ThemeData.light(),
      darkTheme: ThemeData.dark(),
      themeMode: _themeMode,
      home: NotificationListScreen(toggleTheme: _toggleTheme),
    );
  }
}
