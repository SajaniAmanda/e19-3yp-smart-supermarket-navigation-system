import 'package:navigation/pixel.dart';
import 'package:flutter/material.dart';
import 'package:navigation/player.dart';

class HomePage extends StatefulWidget {
  final Stream<String> directionStream;

  HomePage({Key? key, required this.directionStream}) : super(key: key);

  @override
  _HomePageState createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  static int numberInRow = 11;
  int numberOfSquares = numberInRow * 19;

  //palyer position
  int player = 181;

  List<int> barriers = [
    176,
    177,
    178,
    179,
    186,
    185,
    184,
    183,
    168,
    157,
    172,
    161,
    155,
    144,
    133,
    122,
    111,
    100,
    108,
    119,
    130,
    141,
    152,
    163,
    135,
    136,
    137,
    138,
    139,
    124,
    125,
    126,
    127,
    128,
    92,
    81,
    70,
    59,
    48,
    37,
    38,
    49,
    60,
    71,
    82,
    93,
    94,
    83,
    72,
    61,
    50,
    39,
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    12,
    23,
    34,
    20,
    31,
    42
  ];

  @override
  Widget build(BuildContext context) {
    return StreamBuilder<String>(
      stream: widget.directionStream,
      builder: (BuildContext context, AsyncSnapshot<String> snapshot) {
        if (snapshot.hasData) {
          // Update the class-level player variable directly
          player = int.parse(snapshot.data ?? '0');
          // You can call movePlayer here if you need additional logic
        }

        return Scaffold(
          backgroundColor: Colors.black,
          body: Column(
            children: [
              Expanded(
                flex: 4,
                child: GridView.builder(
                  physics: const NeverScrollableScrollPhysics(),
                  itemCount: numberOfSquares,
                  gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(
                    crossAxisCount: numberInRow,
                  ),
                  itemBuilder: (BuildContext context, int index) {
                    if (player == index) {
                      return Container(
                        color: Colors
                            .grey, // Replace with your desired background color
                        child: MyPlayer(),
                      );
                    } else if (barriers.contains(index)) {
                      return MyPixel(
                        color: Colors.blue[700],
                        child: Text(index.toString()),
                      );
                    } else {
                      return MyPixel(
                        color: Colors.grey,
                        child: Text(index.toString()),
                      );
                    }
                  },
                ),
              ),
              Expanded(
                child: Container(
                  child: Column(
                    children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          Text(
                            "Bar code scanner",
                            style: TextStyle(
                              color: Colors.white,
                              fontSize: 24,
                            ),
                          )
                        ],
                      ),
                      Image.asset('assets/images/download.png'),
                    ],
                  ),
                ),
              ),
            ],
          ),
        );
      },
    );
  }
}
