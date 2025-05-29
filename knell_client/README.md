# knell_client

A new Flutter project.

## Getting Started

### Flutter things & notes

Getting the required Hive dependencies (the xplat storage solution)
```
flutter pub get
```

#### knell_client notes

It is going to leverage Hive for cross platform local storage. 

Check the model folder for the interface definition (class) of different structured objects that will be stored. At present the only one that exists is a simple 
**KnellNotification** with the following structure:
```
{
  final String title;
  final String message;
  final String time;
}
```
In the future the datatype for timestampts will change to a UTC representation but for now and simple testing keeping all fields as strings will suffice.

Hive operates in a similar manner to protobufs in that you generate the adapter code after providing a class interface definition. You execute the following to generate the knell_notification.g.dart file that contains the adapter code:

```
flutter pub run build_runner build
```
