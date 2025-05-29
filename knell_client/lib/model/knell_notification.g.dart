// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'knell_notification.dart';

// **************************************************************************
// TypeAdapterGenerator
// **************************************************************************

class KnellNotificationAdapter extends TypeAdapter<KnellNotification> {
  @override
  final int typeId = 0;

  @override
  KnellNotification read(BinaryReader reader) {
    final numOfFields = reader.readByte();
    final fields = <int, dynamic>{
      for (int i = 0; i < numOfFields; i++) reader.readByte(): reader.read(),
    };
    return KnellNotification(
      title: fields[0] as String,
      message: fields[1] as String,
      time: fields[2] as String,
    );
  }

  @override
  void write(BinaryWriter writer, KnellNotification obj) {
    writer
      ..writeByte(3)
      ..writeByte(0)
      ..write(obj.title)
      ..writeByte(1)
      ..write(obj.message)
      ..writeByte(2)
      ..write(obj.time);
  }

  @override
  int get hashCode => typeId.hashCode;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is KnellNotificationAdapter &&
          runtimeType == other.runtimeType &&
          typeId == other.typeId;
}
