import 'dart:async';
import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';
import 'package:google_maps_flutter/google_maps_flutter.dart';
import 'package:http/http.dart' as http;
import 'package:lottie/lottie.dart' hide Marker;

const String kBaseUrl =
    'http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080';
const int kPatientId = 1;

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Google Maps Demo',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blue),
        useMaterial3: true,
      ),
      debugShowCheckedModeBanner: false,
      home: const MapScreen(),
    );
  }
}

class MapScreen extends StatefulWidget {
  const MapScreen({super.key});

  @override
  State<MapScreen> createState() => _MapScreenState();
}

class _MapScreenState extends State<MapScreen> {
  static const CameraPosition _initialPosition = CameraPosition(
    target: LatLng(42.69751, 23.32415), // San Francisco
    zoom: 12,
  );

  int _currentIndex = 0;
  GoogleMapController? _mapController;
  String? _mapStyle;
  Set<Marker> _markers = {};
  Timer? _locationTimer;

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    if (_mapStyle == null) {
      DefaultAssetBundle.of(context).loadString('assets/map_style.json').then((style) {
        setState(() => _mapStyle = style);
      });
    }
  }

  @override
  void initState() {
    super.initState();
    _fetchPatientLocation();
    _locationTimer = Timer.periodic(
      const Duration(seconds: 10),
      (_) => _fetchPatientLocation(),
    );
  }

  Future<void> _fetchPatientLocation() async {
    try {
      final response = await http.get(
        Uri.parse('$kBaseUrl/location/$kPatientId?limit=1'),
      );
      final body = jsonDecode(response.body);
      final data = body['data'] as List?;
      if (body['success'] == true && data != null && data.isNotEmpty) {
        final lat = (data[0]['lat'] as num).toDouble();
        final lng = (data[0]['lng'] as num).toDouble();
        final position = LatLng(lat, lng);
        setState(() {
          _markers = {
            Marker(
              markerId: const MarkerId('patient'),
              position: position,
              infoWindow: const InfoWindow(title: 'Patient'),
            ),
          };
        });
        _mapController?.animateCamera(CameraUpdate.newLatLng(position));
      }
    } catch (_) {}
  }

  void _onMapCreated(GoogleMapController controller) {
    _mapController = controller;
  }

  @override
  void dispose() {
    _locationTimer?.cancel();
    _mapController?.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final List<Widget> pages = [
      GoogleMap(
        initialCameraPosition: _initialPosition,
        onMapCreated: _onMapCreated,
        myLocationButtonEnabled: false,
        style: _mapStyle,
        markers: _markers,
      ),
      const MedPage(),
    ];

    return Scaffold(
      bottomNavigationBar: BottomNavigationBar(
        items: const [
          BottomNavigationBarItem(
            icon: Padding(
              padding: EdgeInsets.only(top: 4),
              child: Icon(Icons.home_outlined),
            ),
            label: 'Home',
          ),
          BottomNavigationBarItem(
            icon: Padding(
              padding: EdgeInsets.only(top: 4),
              child: Icon(CupertinoIcons.bag_fill_badge_plus),
            ),
            label: 'Med',
          ),
        ],
        currentIndex: _currentIndex,
        onTap: (index) {
          setState(() {
            _currentIndex = index;
          });
        },
        backgroundColor: Colors.black,
        selectedItemColor: Colors.white,
        unselectedItemColor: Colors.grey,
        selectedLabelStyle: TextStyle(
          fontWeight: FontWeight.w600,
          fontSize: 14
        ),
      ),
      body: pages[_currentIndex],
    );
  }
}

class MedPage extends StatefulWidget {
  const MedPage({super.key});

  @override
  State<MedPage> createState() => _MedPageState();
}

class _MedPageState extends State<MedPage> {

  String? _eventType;
  bool _loading = true;

  @override
  void initState() {
    super.initState();
    _fetchLatestEvent();
  }

  Future<void> _fetchLatestEvent() async {
    try {
      final response = await http.get(
        Uri.parse('$kBaseUrl/medication/$kPatientId?limit=1'),
      );
      final body = jsonDecode(response.body);
      final data = body['data'] as List?;
      if (body['success'] == true && data != null && data.isNotEmpty) {
        setState(() {
          _eventType = data[0]['eventType'] as String?;
        });
      }
    } catch (_) {}
    setState(() => _loading = false);
  }

  @override
  Widget build(BuildContext context) {
    final bool doseTaken = _eventType == 'DOSE_TAKEN';
    return Scaffold(
      backgroundColor: Colors.white,
      body: Center(
        child: _loading
            ? const CircularProgressIndicator()
            : Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Text(
                    'Pills taken?',
                    style: TextStyle(fontSize: 20, fontWeight: FontWeight.w600),
                  ),
                  Lottie.asset(
                    doseTaken
                        ? 'assets/Success Check.json'
                        : 'assets/Tomato Error.json',
                    width: 300,
                    height: 300,
                    repeat: false,
                  ),
                  Text(
                    doseTaken
                        ? 'Amazing!'
                        : 'Oh no! Better go take them!',
                    style: TextStyle(fontSize: 20, fontWeight: FontWeight.w600),
                  ),
                ],
              ),
      ),
    );
  }
}
