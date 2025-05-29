import 'package:flutter/material.dart';
import 'package:hive_flutter/hive_flutter.dart';
import '../model/knell_notification.dart';
import '../components/notification_tile.dart';

class NotificationListScreen extends StatefulWidget {
  final VoidCallback toggleTheme;
  const NotificationListScreen({super.key, required this.toggleTheme});

  @override
  NotificationListScreenState createState() => NotificationListScreenState();
}

class NotificationListScreenState extends State<NotificationListScreen> {
  final Box<KnellNotification> box = Hive.box<KnellNotification>(
    'notifications',
  );

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Notifications'),
        actions: [
          IconButton(
            icon: const Icon(Icons.brightness_6),
            onPressed: widget.toggleTheme,
          ),
        ],
      ),
      body: ValueListenableBuilder(
        valueListenable: box.listenable(),
        builder: (context, Box<KnellNotification> notificationsBox, _) {
          if (notificationsBox.isEmpty) {
            return const Center(child: Text('No notifications'));
          }

          return ListView.builder(
            itemCount: notificationsBox.length,
            itemBuilder: (context, index) {
              final notification = notificationsBox.getAt(index)!;

              return NotificationTile(
                key: ValueKey(notification.time + notification.title),
                notification: notification,
                onDismissed: () {
                  notificationsBox.deleteAt(index);
                },
              );
            },
          );
        },
      ),
    );
  }
}
