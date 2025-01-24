import 'package:flutter/material.dart';
import 'package:trackmate_ui/learn_flutter_page.dart';

class HomePage extends StatelessWidget {
  const HomePage({super.key});

  @override
  Widget build(BuildContext context) {
    return Center(
      child: ElevatedButton(
          onPressed: () {
            Navigator.of(context)
                .push(MaterialPageRoute(builder: (BuildContext context) {
              return const LearnFlutterPage();
            }));
          },
          child: Text("Learning Flutter Nigga")),
    );
  }
}
