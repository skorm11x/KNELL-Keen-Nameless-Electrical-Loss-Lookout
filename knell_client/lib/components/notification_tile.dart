import 'package:flutter/material.dart';
import '../model/knell_notification.dart';
import '../screens/notification_detail.dart';
import '../helper/feature_check.dart';

class NotificationTile extends StatefulWidget {
  final KnellNotification notification;
  final VoidCallback? onDismissed;

  const NotificationTile({
    super.key,
    required this.notification,
    this.onDismissed,
  });

  @override
  // ignore: library_private_types_in_public_api
  _NotificationTileState createState() => _NotificationTileState();
}

class _NotificationTileState extends State<NotificationTile> {
  bool _isHovering = false;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    Widget trailingWidget = canHover
        ? MouseRegion(
            onEnter: (_) => setState(() => _isHovering = true),
            onExit: (_) => setState(() => _isHovering = false),
            child: InkWell(
              borderRadius: BorderRadius.circular(8),
              onTap: () {
                if (widget.onDismissed != null) {
                  widget.onDismissed!();
                }
              },
              child: Card(
                elevation: _isHovering ? 8 : 2,
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(8),
                  side: BorderSide(
                    color: _isHovering ? Colors.blue : Colors.transparent,
                    width: 1.5,
                  ),
                ),
                child: Padding(
                  padding: const EdgeInsets.symmetric(
                    horizontal: 8,
                    vertical: 4,
                  ),
                  child: Column(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      Icon(
                        Icons.delete,
                        color: _isHovering
                            ? Colors.blue
                            : theme.iconTheme.color,
                        size: 24,
                      ),
                      const SizedBox(height: 4),
                    ],
                  ),
                ),
              ),
            ),
          )
        : Text(widget.notification.time);

    Widget tile = ListTile(
      // leading: const Icon(Icons.notifications),
      title: Text("${widget.notification.title} : ${widget.notification.time}"),
      subtitle: Text(widget.notification.message),
      trailing: trailingWidget,
      onTap: () {
        Navigator.push(
          context,
          MaterialPageRoute(
            builder: (context) =>
                NotificationDetailScreen(notification: widget.notification),
          ),
        );
      },
    );

    if (!canHover && widget.onDismissed != null) {
      return Dismissible(
        key:
            widget.key ??
            ValueKey(widget.notification.time + widget.notification.title),
        direction: DismissDirection.endToStart,
        onDismissed: (_) => widget.onDismissed!(),
        background: Container(
          color: Colors.red,
          alignment: Alignment.centerRight,
          padding: const EdgeInsets.symmetric(horizontal: 20),
          child: const Icon(Icons.delete, color: Colors.white),
        ),
        child: tile,
      );
    } else {
      return tile;
    }
  }
}
