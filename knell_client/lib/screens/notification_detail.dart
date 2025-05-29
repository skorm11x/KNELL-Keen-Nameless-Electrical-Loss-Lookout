import 'package:flutter/material.dart';
import '../model/knell_notification.dart';

class NotificationDetailScreen extends StatelessWidget {
  final KnellNotification notification;

  const NotificationDetailScreen({super.key, required this.notification});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text(notification.title)),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(notification.message, style: const TextStyle(fontSize: 18)),
            const SizedBox(height: 20),
            Text('Time: ${notification.time}'),
          ],
        ),
      ),
    );
  }
}
