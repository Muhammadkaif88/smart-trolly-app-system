import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:intl/intl.dart';
import 'package:printing/printing.dart';
import 'package:pdf/pdf.dart';
import 'package:pdf/widgets.dart' as pw;
import 'staff_dashboard.dart';
import 'settings_page.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  await Firebase.initializeApp();

  runApp(const SmartTrolleyApp());
}

class SmartTrolleyApp extends StatelessWidget {
  const SmartTrolleyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Smart Trolley',
      theme: ThemeData.dark(),
      home: StaffDashboard(),
    );
  }
}

class HomePage extends StatefulWidget {
  const HomePage({super.key});

  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  final DatabaseReference trolleyRef = FirebaseDatabase.instance.ref(
    'TROLLEY-1',
  );

  Map data = {};

  @override
  void initState() {
    super.initState();

    trolleyRef.onValue.listen((event) {
      final dbData = event.snapshot.value;

      if (dbData != null) {
        setState(() {
          data = Map.from(dbData as Map);
        });
      }
    });
  }

  Widget productTile(
    String name,
    int price,
    int qty,
    int total,
    IconData icon,
    Color color,
  ) {
    return Container(
      margin: const EdgeInsets.only(bottom: 14),
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.05),
        borderRadius: BorderRadius.circular(20),
      ),
      child: Row(
        children: [
          CircleAvatar(
            radius: 26,
            backgroundColor: color.withOpacity(0.2),
            child: Icon(icon, color: color),
          ),

          const SizedBox(width: 14),

          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                Text(
                  name,
                  style: GoogleFonts.poppins(
                    fontSize: 18,
                    fontWeight: FontWeight.w600,
                  ),
                ),

                const SizedBox(height: 4),

                Text(
                  '₹$price x $qty',
                  style: GoogleFonts.poppins(
                    color: Colors.white70,
                    fontSize: 13,
                  ),
                ),
              ],
            ),
          ),

          Text(
            '₹$total',
            style: GoogleFonts.poppins(
              fontSize: 18,
              fontWeight: FontWeight.bold,
              color: Colors.greenAccent,
            ),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final items = data['items'] ?? {};

    double subtotal = (data['subtotal'] ?? 0).toDouble();

    double gst = (data['gst'] ?? 0).toDouble();

    double grandTotal = (data['grandTotal'] ?? 0).toDouble();

    final screenSize = MediaQuery.of(context).size;
    final isMobile = screenSize.width < 600;
    final responsivePadding = isMobile ? 12.0 : 16.0;
    final titleFontSize = isMobile ? 18.0 : 24.0;

    return Scaffold(
      backgroundColor: const Color(0xff09090F),

      appBar: AppBar(
        backgroundColor: Colors.transparent,
        elevation: 0,
        title: Text(
          'SMART TROLLEY',
          style: GoogleFonts.poppins(
            fontWeight: FontWeight.bold,
            letterSpacing: 1,
            fontSize: titleFontSize,
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
        child: SingleChildScrollView(
          child: Column(
            children: [
              // ==========================================
              // HEADER CARD
              // ==========================================
              Container(
                width: double.infinity,
                padding: EdgeInsets.all(isMobile ? 16 : 24),
                decoration: BoxDecoration(
                  gradient: const LinearGradient(
                    colors: [Color(0xff5B86E5), Color(0xff36D1DC)],
                  ),
                  borderRadius: BorderRadius.circular(30),
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      'TROLLEY-1',
                      style: GoogleFonts.poppins(
                        fontSize: isMobile ? 20 : 28,
                        fontWeight: FontWeight.bold,
                      ),
                    ),

                    const SizedBox(height: 8),

                    Text(
                      DateFormat(
                        'dd MMM yyyy • hh:mm a',
                      ).format(DateTime.now()),
                      style: GoogleFonts.poppins(fontSize: isMobile ? 12 : 14),
                    ),

                    const SizedBox(height: 20),

                    Row(
                      children: [
                        Expanded(
                          child: statCard(
                            'Subtotal',
                            '₹${subtotal.toStringAsFixed(2)}',
                          ),
                        ),

                        const SizedBox(width: 12),

                        Expanded(
                          child: statCard('GST', '₹${gst.toStringAsFixed(2)}'),
                        ),
                      ],
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 25),

              // ==========================================
              // PRODUCT LIST
              // ==========================================
              Align(
                alignment: Alignment.centerLeft,
                child: Text(
                  'Purchased Items',
                  style: GoogleFonts.poppins(
                    fontSize: 20,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ),

              const SizedBox(height: 18),

              if (items['Milk'] != null)
                productTile(
                  'Milk',
                  items['Milk']['price'] ?? 0,
                  items['Milk']['qty'] ?? 0,
                  items['Milk']['total'] ?? 0,
                  Icons.local_drink,
                  Colors.blue,
                ),

              if (items['Soap'] != null)
                productTile(
                  'Soap',
                  items['Soap']['price'] ?? 0,
                  items['Soap']['qty'] ?? 0,
                  items['Soap']['total'] ?? 0,
                  Icons.soap,
                  Colors.orange,
                ),

              if (items['Biscuit'] != null)
                productTile(
                  'Biscuit',
                  items['Biscuit']['price'] ?? 0,
                  items['Biscuit']['qty'] ?? 0,
                  items['Biscuit']['total'] ?? 0,
                  Icons.cookie,
                  Colors.brown,
                ),

              const SizedBox(height: 20),

              // ==========================================
              // CHECKOUT CARD
              // ==========================================
              Container(
                padding: const EdgeInsets.all(24),
                decoration: BoxDecoration(
                  color: Colors.white.withOpacity(0.05),
                  borderRadius: BorderRadius.circular(25),
                ),
                child: Column(
                  children: [
                    rowData('Subtotal', subtotal),

                    const SizedBox(height: 12),

                    rowData('GST 5%', gst),

                    const Divider(height: 30),

                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Text(
                          'TOTAL',
                          style: GoogleFonts.poppins(
                            fontSize: isMobile ? 16 : 22,
                            fontWeight: FontWeight.bold,
                          ),
                        ),

                        Text(
                          '₹${grandTotal.toStringAsFixed(2)}',
                          style: GoogleFonts.poppins(
                            fontSize: isMobile ? 20 : 28,
                            fontWeight: FontWeight.bold,
                            color: Colors.greenAccent,
                          ),
                        ),
                      ],
                    ),

                    const SizedBox(height: 25),

                    const SizedBox(height: 20),

                    SizedBox(
                      width: double.infinity,
                      height: 60,
                      child: ElevatedButton(
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.greenAccent,
                          foregroundColor: Colors.black,
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(20),
                          ),
                        ),
                        onPressed: () {
                          Navigator.push(
                            context,
                            MaterialPageRoute(
                              builder: (_) => QRCodePaymentPage(
                                grandTotal: grandTotal,
                                items: items,
                                subtotal: subtotal,
                                gst: gst,
                              ),
                            ),
                          );
                        },
                        child: Text(
                          'CHECKOUT',
                          style: GoogleFonts.poppins(
                            fontSize: 18,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 40),
            ],
          ),
        ),
      ),
    );
  }

  Widget statCard(String title, String value) {
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: Colors.white.withOpacity(0.15),
        borderRadius: BorderRadius.circular(18),
      ),
      child: Column(
        children: [
          Text(title, style: GoogleFonts.poppins(color: Colors.white70)),

          const SizedBox(height: 10),

          Text(
            value,
            style: GoogleFonts.poppins(
              fontWeight: FontWeight.bold,
              fontSize: 20,
            ),
          ),
        ],
      ),
    );
  }

  Widget rowData(String title, double value) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        Text(
          title,
          style: GoogleFonts.poppins(fontSize: 16, color: Colors.white70),
        ),

        Text(
          '₹${value.toStringAsFixed(2)}',
          style: GoogleFonts.poppins(fontSize: 18, fontWeight: FontWeight.w600),
        ),
      ],
    );
  }
}

class QRCodePaymentPage extends StatefulWidget {
  final double grandTotal;
  final Map items;
  final double subtotal;
  final double gst;

  const QRCodePaymentPage({
    super.key,
    required this.grandTotal,
    this.items = const {},
    this.subtotal = 0.0,
    this.gst = 0.0,
  });

  @override
  State<QRCodePaymentPage> createState() => _QRCodePaymentPageState();
}

class _QRCodePaymentPageState extends State<QRCodePaymentPage> {
  bool isPaymentComplete = false;

  Future<void> _generateAndDownloadInvoice() async {
    final pdf = pw.Document();

    pdf.addPage(
      pw.Page(
        pageFormat: PdfPageFormat.a4,
        build: (pw.Context context) {
          return pw.Column(
            crossAxisAlignment: pw.CrossAxisAlignment.start,
            children: [
              pw.Center(
                child: pw.Text(
                  'SMART TROLLEY INVOICE',
                  style: pw.TextStyle(
                    fontSize: 24,
                    fontWeight: pw.FontWeight.bold,
                  ),
                ),
              ),
              pw.SizedBox(height: 20),
              pw.Text(
                'Trolley: TROLLEY-1',
                style: const pw.TextStyle(fontSize: 12),
              ),
              pw.Text(
                'Date: ${DateFormat('dd MMM yyyy • hh:mm a').format(DateTime.now())}',
                style: const pw.TextStyle(fontSize: 12),
              ),
              pw.SizedBox(height: 20),
              pw.Text(
                'Items Purchased:',
                style: pw.TextStyle(
                  fontSize: 14,
                  fontWeight: pw.FontWeight.bold,
                ),
              ),
              pw.SizedBox(height: 10),
              pw.Table(
                border: pw.TableBorder.all(),
                children: [
                  pw.TableRow(
                    children: [
                      pw.Padding(
                        padding: const pw.EdgeInsets.all(8),
                        child: pw.Text(
                          'Item',
                          style: pw.TextStyle(fontWeight: pw.FontWeight.bold),
                        ),
                      ),
                      pw.Padding(
                        padding: const pw.EdgeInsets.all(8),
                        child: pw.Text(
                          'Price',
                          style: pw.TextStyle(fontWeight: pw.FontWeight.bold),
                        ),
                      ),
                      pw.Padding(
                        padding: const pw.EdgeInsets.all(8),
                        child: pw.Text(
                          'Qty',
                          style: pw.TextStyle(fontWeight: pw.FontWeight.bold),
                        ),
                      ),
                      pw.Padding(
                        padding: const pw.EdgeInsets.all(8),
                        child: pw.Text(
                          'Total',
                          style: pw.TextStyle(fontWeight: pw.FontWeight.bold),
                        ),
                      ),
                    ],
                  ),
                  if (widget.items['Milk'] != null)
                    pw.TableRow(
                      children: [
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text('Milk'),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text(
                            '₹${widget.items['Milk']['price'] ?? 0}',
                          ),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text('${widget.items['Milk']['qty'] ?? 0}'),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text(
                            '₹${widget.items['Milk']['total'] ?? 0}',
                          ),
                        ),
                      ],
                    ),
                  if (widget.items['Soap'] != null)
                    pw.TableRow(
                      children: [
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text('Soap'),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text(
                            '₹${widget.items['Soap']['price'] ?? 0}',
                          ),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text('${widget.items['Soap']['qty'] ?? 0}'),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text(
                            '₹${widget.items['Soap']['total'] ?? 0}',
                          ),
                        ),
                      ],
                    ),
                  if (widget.items['Biscuit'] != null)
                    pw.TableRow(
                      children: [
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text('Biscuit'),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text(
                            '₹${widget.items['Biscuit']['price'] ?? 0}',
                          ),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text(
                            '${widget.items['Biscuit']['qty'] ?? 0}',
                          ),
                        ),
                        pw.Padding(
                          padding: const pw.EdgeInsets.all(8),
                          child: pw.Text(
                            '₹${widget.items['Biscuit']['total'] ?? 0}',
                          ),
                        ),
                      ],
                    ),
                ],
              ),
              pw.SizedBox(height: 20),
              pw.Row(
                mainAxisAlignment: pw.MainAxisAlignment.end,
                children: [
                  pw.Column(
                    crossAxisAlignment: pw.CrossAxisAlignment.start,
                    children: [
                      pw.Text(
                        'Subtotal: ₹${widget.subtotal.toStringAsFixed(2)}',
                        style: const pw.TextStyle(fontSize: 12),
                      ),
                      pw.Text(
                        'GST (5%): ₹${widget.gst.toStringAsFixed(2)}',
                        style: const pw.TextStyle(fontSize: 12),
                      ),
                      pw.Divider(),
                      pw.Text(
                        'Total: ₹${widget.grandTotal.toStringAsFixed(2)}',
                        style: pw.TextStyle(
                          fontSize: 14,
                          fontWeight: pw.FontWeight.bold,
                        ),
                      ),
                    ],
                  ),
                ],
              ),
            ],
          );
        },
      ),
    );

    await Printing.sharePdf(
      bytes: await pdf.save(),
      filename: 'invoice_${DateTime.now().millisecondsSinceEpoch}.pdf',
    );
  }

  @override
  Widget build(BuildContext context) {
    final screenSize = MediaQuery.of(context).size;
    final isMobile = screenSize.width < 600;

    return Scaffold(
      backgroundColor: const Color(0xff09090F),
      appBar: AppBar(
        backgroundColor: Colors.transparent,
        elevation: 0,
        title: Text(
          'PAYMENT',
          style: GoogleFonts.poppins(
            fontWeight: FontWeight.bold,
            letterSpacing: 1,
          ),
        ),
        centerTitle: true,
        leading: IconButton(
          icon: const Icon(Icons.arrow_back),
          onPressed: () => Navigator.pop(context),
        ),
      ),
      body: Padding(
        padding: EdgeInsets.all(isMobile ? 16 : 24),
        child: SingleChildScrollView(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Container(
                padding: EdgeInsets.all(isMobile ? 24 : 40),
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius: BorderRadius.circular(30),
                ),
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Icon(
                      Icons.qr_code_2,
                      size: isMobile ? 160 : 240,
                      color: Colors.black,
                    ),
                    const SizedBox(height: 20),
                    Text(
                      'Scan & Pay',
                      style: GoogleFonts.poppins(
                        color: Colors.black,
                        fontWeight: FontWeight.bold,
                        fontSize: isMobile ? 18 : 24,
                      ),
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 30),
              Container(
                padding: EdgeInsets.all(isMobile ? 16 : 20),
                decoration: BoxDecoration(
                  color: Colors.white.withOpacity(0.1),
                  borderRadius: BorderRadius.circular(20),
                ),
                child: Column(
                  children: [
                    Text(
                      'Total Amount',
                      style: GoogleFonts.poppins(
                        color: Colors.white70,
                        fontSize: isMobile ? 14 : 16,
                      ),
                    ),
                    const SizedBox(height: 10),
                    Text(
                      '₹${widget.grandTotal.toStringAsFixed(2)}',
                      style: GoogleFonts.poppins(
                        fontSize: isMobile ? 28 : 40,
                        fontWeight: FontWeight.bold,
                        color: Colors.greenAccent,
                      ),
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 30),
              if (!isPaymentComplete)
                SizedBox(
                  width: double.infinity,
                  height: 56,
                  child: ElevatedButton(
                    style: ElevatedButton.styleFrom(
                      backgroundColor: Colors.greenAccent,
                      foregroundColor: Colors.black,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(20),
                      ),
                    ),
                    onPressed: () async {
                      // Send reset flag to ESP32 and clear app data
                      final dbRef = FirebaseDatabase.instance.ref('TROLLEY-1');
                      await dbRef.child('reset').set(true);
                      await dbRef.child('items').remove();
                      await dbRef.child('subtotal').set(0);
                      await dbRef.child('gst').set(0);
                      await dbRef.child('grandTotal').set(0);

                      setState(() {
                        isPaymentComplete = true;
                      });
                      
                      if (mounted) {
                        ScaffoldMessenger.of(context).showSnackBar(
                          const SnackBar(content: Text('Payment Successful! Trolley reset for next customer.')),
                        );
                      }
                    },
                    child: Text(
                      'PAYMENT DONE',
                      style: GoogleFonts.poppins(
                        fontSize: isMobile ? 16 : 18,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                  ),
                ),
              if (isPaymentComplete)
                Column(
                  children: [
                    const Icon(
                      Icons.check_circle,
                      size: 60,
                      color: Colors.greenAccent,
                    ),
                    const SizedBox(height: 20),
                    Text(
                      'Payment Successful!',
                      style: GoogleFonts.poppins(
                        fontSize: isMobile ? 18 : 22,
                        fontWeight: FontWeight.bold,
                        color: Colors.greenAccent,
                      ),
                    ),
                    const SizedBox(height: 30),
                    SizedBox(
                      width: double.infinity,
                      height: 60,
                      child: ElevatedButton(
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.amber,
                          foregroundColor: Colors.black,
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(20),
                          ),
                        ),
                        onPressed: _generateAndDownloadInvoice,
                        child: Text(
                          'DOWNLOAD INVOICE',
                          style: GoogleFonts.poppins(
                            fontSize: isMobile ? 14 : 18,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ),
                    ),
                    const SizedBox(height: 20),
                    SizedBox(
                      width: double.infinity,
                      height: 60,
                      child: ElevatedButton(
                        style: ElevatedButton.styleFrom(
                          backgroundColor: Colors.blue,
                          foregroundColor: Colors.white,
                          shape: RoundedRectangleBorder(
                            borderRadius: BorderRadius.circular(20),
                          ),
                        ),
                        onPressed: () {
                          Navigator.of(
                            context,
                          ).popUntil((route) => route.isFirst);
                        },
                        child: Text(
                          'BACK TO HOME',
                          style: GoogleFonts.poppins(
                            fontSize: isMobile ? 14 : 18,
                            fontWeight: FontWeight.bold,
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
            ],
          ),
        ),
      ),
    );
  }
}
