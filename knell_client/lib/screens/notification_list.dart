import 'package:flutter/material.dart';
import 'notification_detail.dart';
import '../model/knell_notification.dart';

class NotificationListScreen extends StatefulWidget {
  final VoidCallback toggleTheme;
  const NotificationListScreen({super.key, required this.toggleTheme});

  @override
  NotificationListScreenState createState() => NotificationListScreenState();
}

class NotificationListScreenState extends State<NotificationListScreen> {
  List<KnellNotification> notifications = [
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
  ];

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
      body: ListView.builder(
        itemCount: notifications.length,
        itemBuilder: (context, index) {
          final notification = notifications[index];
          return ListTile(
            leading: const Icon(Icons.notifications),
            title: Text(notification.title),
            subtitle: Text(notification.message),
            trailing: Text(notification.time),
            onTap: () {
              Navigator.push(
                context,
                MaterialPageRoute(
                  builder: (context) =>
                      NotificationDetailScreen(notification: notification),
                ),
              );
            },
          );
        },
      ),
    );
  }
}
