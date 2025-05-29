import 'package:hive/hive.dart';

part 'knell_notification.g.dart';

@HiveType(typeId: 0)
class KnellNotification {
  @HiveField(0)
  final String title;
  @HiveField(1)
  final String message;
  @HiveField(2)
  final String time;

  KnellNotification({
    required this.title,
    required this.message,
    required this.time,
  });

  factory KnellNotification.fromMap(Map<String, dynamic> map) {
    return KnellNotification(
      title: map['title'],
      message: map['message'],
      time: map['time'],
    );
  }
}
