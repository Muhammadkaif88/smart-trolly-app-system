import 'package:flutter/material.dart';
import 'main.dart';
import 'settings_page.dart';

class StaffDashboard extends StatelessWidget {
  StaffDashboard({super.key});

  @override
  Widget build(BuildContext context) {
    final screenSize = MediaQuery.of(context).size;
    final isMobile = screenSize.width < 600;
    final isTablet = screenSize.width >= 600 && screenSize.width < 1024;
    final responsivePadding = isMobile ? 12.0 : 16.0;
    final crossAxisCount = isMobile ? 2 : (isTablet ? 3 : 4);

    return Scaffold(
      backgroundColor: const Color(0xff09090F),
      appBar: AppBar(
        title: Text(
          'SHOP STAFF PANEL',
          style: TextStyle(
            fontSize: isMobile ? 18 : 24,
            fontWeight: FontWeight.bold,
          ),
        ),
        centerTitle: true,
        actions: [
          IconButton(
            icon: const Icon(Icons.settings, color: Colors.white),
            onPressed: () {
              Navigator.push(
                context,
                MaterialPageRoute(builder: (_) => const SettingsPage()),
              );
            },
          ),
        ],
      ),
      body: Padding(
        padding: EdgeInsets.all(responsivePadding),
        child: GridView.count(
          crossAxisCount: crossAxisCount,
          childAspectRatio: 0.8,
          crossAxisSpacing: responsivePadding,
          mainAxisSpacing: responsivePadding,
          children: [
            trolleyCard(context, 'TROLLEY-1', Colors.green, true),
            trolleyCard(context, 'TROLLEY-2', Colors.orange, false),
            trolleyCard(context, 'TROLLEY-3', Colors.red, false),
          ],
        ),
      ),
      bottomNavigationBar: _buildBottomNav(context),
    );
  }

  Widget _buildBottomNav(BuildContext context) {
    final screenSize = MediaQuery.of(context).size;
    final isMobile = screenSize.width < 600;
    final responsiveMargin = isMobile ? 12.0 : 15.0;
    final responsivePadding = isMobile ? 8.0 : 10.0;

    return Container(
      margin: EdgeInsets.all(responsiveMargin),
      padding: EdgeInsets.symmetric(
        horizontal: responsivePadding * 2,
        vertical: responsivePadding,
      ),
      decoration: BoxDecoration(
        color: const Color(0xff1A1A25),
        borderRadius: BorderRadius.circular(30),
        boxShadow: const [BoxShadow(color: Colors.black54, blurRadius: 10)],
      ),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceAround,
        children: [
          navItem(Icons.home, true),
          navItem(Icons.shopping_cart, false),
          navItem(Icons.qr_code_scanner, false),
          navItem(Icons.person, false),
        ],
      ),
    );
  }

  Widget trolleyCard(
    BuildContext context,
    String trolleyName,
    Color color,
    bool active,
  ) {
    final screenSize = MediaQuery.of(context).size;
    final isMobile = screenSize.width < 600;
    final cardPadding = isMobile ? 12.0 : 18.0;
    final titleFontSize = isMobile ? 18.0 : 24.0;
    final iconSize = isMobile ? 28.0 : 35.0;
    final statusFontSize = isMobile ? 9.0 : 11.0;

    return GestureDetector(
      onTap: () {
        Navigator.push(
          context,
          MaterialPageRoute(builder: (_) => const HomePage()),
        );
      },
      child: Container(
        padding: EdgeInsets.all(cardPadding),
        decoration: BoxDecoration(
          gradient: LinearGradient(colors: [color, color.withOpacity(0.4)]),
          borderRadius: BorderRadius.circular(30),
        ),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                Icon(Icons.shopping_cart, color: Colors.white, size: iconSize),
                Container(
                  padding: const EdgeInsets.symmetric(
                    horizontal: 10,
                    vertical: 5,
                  ),
                  decoration: BoxDecoration(
                    color: active ? Colors.greenAccent : Colors.black26,
                    borderRadius: BorderRadius.circular(20),
                  ),
                  child: Text(
                    active ? 'ACTIVE' : 'OFFLINE',
                    style: TextStyle(
                      color: Colors.black,
                      fontWeight: FontWeight.bold,
                      fontSize: statusFontSize,
                    ),
                  ),
                ),
              ],
            ),
            const Spacer(),
            Text(
              trolleyName,
              style: TextStyle(
                fontSize: titleFontSize,
                fontWeight: FontWeight.bold,
                color: Colors.white,
              ),
            ),
            const SizedBox(height: 8),
            Row(
              children: [
                const Icon(Icons.wifi, color: Colors.white70, size: 16),
                const SizedBox(width: 6),
                Expanded(
                  child: Text(
                    'Realtime Connected',
                    style: TextStyle(
                      color: Colors.white70,
                      fontSize: isMobile ? 11 : 12,
                    ),
                    overflow: TextOverflow.ellipsis,
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }

  Widget navItem(IconData icon, bool isActive) {
    return GestureDetector(
      onTap: () {
        // Add navigation logic here
      },
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Icon(
            icon,
            color: isActive ? Colors.greenAccent : Colors.grey,
            size: 24,
          ),
        ],
      ),
    );
  }
}
