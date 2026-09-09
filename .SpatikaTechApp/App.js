import React, { useState, useEffect } from 'react';
import { 
  StyleSheet, Text, View, TextInput, TouchableOpacity, 
  KeyboardAvoidingView, ActivityIndicator, Alert,
  StatusBar, Platform, PermissionsAndroid, Linking 
} from 'react-native';
import { WebView } from 'react-native-webview';
import WifiManager from 'react-native-wifi-reborn';

export default function App() {
  const [isAuthenticated, setIsAuthenticated] = useState(false);
  const [username, setUsername] = useState('');
  const [password, setPassword] = useState('');
  const [isConnected, setIsConnected] = useState(false);
  const [isAttemptingLogin, setIsAttemptingLogin] = useState(false);
  const [connectionStatus, setConnectionStatus] = useState("Checking network...");

  const SPATIKA_URL = 'http://192.168.4.1';

  // Request Location permissions (required by Android 10+ to change Wi-Fi)
  const requestLocationPermission = async () => {
    if (Platform.OS === 'android') {
      try {
        const granted = await PermissionsAndroid.request(
          PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
          {
            title: 'Location permission needed',
            message: 'To connect to the Spatika hardware automatically, Location services are required.',
            buttonNegative: 'Cancel',
            buttonPositive: 'OK',
          },
        );
        return granted === PermissionsAndroid.RESULTS.GRANTED;
      } catch (err) {
        console.warn(err);
        return false;
      }
    }
    return true; // iOS handles natively inside OS popup
  };

  // Attempt local authentication & force Wi-Fi connection
  const handleLogin = async () => {
    if (username.trim() !== 'SpatikaWeb' || password !== 'Spatika123') {
      Alert.alert("Authentication Failed", "Incorrect Technician Credentials.");
      return;
    }

    setIsAttemptingLogin(true);

    const hasLocation = await requestLocationPermission();
    if (!hasLocation) {
      Alert.alert("Permission Error", "Cannot auto-connect to station without location permission.");
      setIsAttemptingLogin(false);
      return;
    }

    try {
      setConnectionStatus("Routing hardware interface...");
      setIsAuthenticated(true); // Drop into the dashboard first for UI fluidity
      
      // Fire-and-forget native auto-connect (prevents Promise deadlock on Android 11+)
      WifiManager.connectToProtectedSSID('SpatikaWeb', 'Spatika123', false, false)
        .catch(err => console.log('Wi-Fi Native Background Error:', err));
      
      // Unconditionally show WebView interface after 2 seconds!
      setTimeout(() => {
        setIsAttemptingLogin(false);
        setIsConnected(true);
      }, 2000);

    } catch (error) {
      setIsAttemptingLogin(false);
      setIsConnected(false); // It fell through to the fallback overlay
      console.log('Wi-Fi Native Error:', error);
    }
  };

  // 1. Auth Gateway Screen
  if (!isAuthenticated) {
    return (
      <View style={styles.loginContainer}>
        <StatusBar barStyle="light-content" />
        <View style={styles.headerBox}>
          <Text style={styles.logoText}>SPATIKA</Text>
          <Text style={styles.subLogoText}>TECHNICIAN PORTAL</Text>
        </View>

        <View style={styles.formContainer}>
          <Text style={styles.label}>Station User ID</Text>
          <TextInput
            style={styles.input}
            placeholder="e.g. SpatikaWeb"
            placeholderTextColor="#888"
            autoCapitalize="none"
            value={username}
            onChangeText={setUsername}
          />
          
          <Text style={styles.label}>Passcode</Text>
          <TextInput
            style={styles.input}
            placeholder="••••••••"
            placeholderTextColor="#888"
            secureTextEntry
            value={password}
            onChangeText={setPassword}
          />

          <TouchableOpacity style={styles.loginButton} onPress={handleLogin} disabled={isAttemptingLogin}>
            {isAttemptingLogin ? (
              <ActivityIndicator color="#fff" />
            ) : (
              <Text style={styles.loginButtonText}>SECURE LOGIN</Text>
            )}
          </TouchableOpacity>
        </View>
        
        <Text style={styles.footerText}>v1.0.0 Firmware Access Tool</Text>
      </View>
    );
  }

  // 2. Main Dashboard (Wrapper around ESP32 Local Server)
  return (
    <View style={styles.dashboardContainer}>
      <StatusBar barStyle="dark-content" />
      <View style={styles.dashHeader}>
        <Text style={styles.dashTitle}>Station Web Control</Text>
        
        <View style={{flexDirection: 'row', alignItems: 'center'}}>
          <TouchableOpacity 
             onPress={() => Linking.openURL(SPATIKA_URL)}
             style={{backgroundColor: '#0284c7', paddingHorizontal: 10, paddingVertical: 5, borderRadius: 5, marginRight: 15}}
          >
            <Text style={{color: '#fff', fontWeight: '600', fontSize: 13}}>Chrome</Text>
          </TouchableOpacity>
          <TouchableOpacity onPress={() => setIsAuthenticated(false)}>
            <Text style={styles.logoutText}>Logout</Text>
          </TouchableOpacity>
        </View>
      </View>
      
      {!isConnected ? (
         <View style={styles.connectingOverlay}>
            <ActivityIndicator size="large" color="#007bff" />
            <Text style={styles.overlayTitle}>{connectionStatus}</Text>
            <Text style={styles.overlaySub}>Please ensure you are close to</Text>
            <Text style={styles.highlightText}>SpatikaWeb</Text>
            <Text style={styles.overlaySub}>or click to login directly from your browser.</Text>
         </View>
      ) : (
        <View style={{ flex: 1 }}>
          <WebView 
            // the React `key` forces WebView to entirely mount from scratch if it changes!
            key={connectionStatus} 
            source={{ uri: SPATIKA_URL }} 
            style={styles.webview}
            javaScriptEnabled={true}
            domStorageEnabled={true}
            startInLoadingState={true}
            mixedContentMode="always"
            renderError={() => (
              <View style={styles.connectingOverlay}>
                <Text style={{color: 'red', fontSize: 18, fontWeight: 'bold'}}>Connection Failed.</Text>
                <Text style={{color: 'gray', marginTop: 10, textAlign: 'center'}}>
                   Android routed this request to your Cellular Data because it thinks the local Wi-Fi has no internet.
                </Text>
                <TouchableOpacity 
                   style={{...styles.loginButton, marginTop: 20}} 
                   onPress={() => setConnectionStatus(Date.now().toString())}
                >
                   <Text style={styles.loginButtonText}>FORCE RETRY 192.168.4.1</Text>
                </TouchableOpacity>
              </View>
            )}
          />
        </View>
      )}
    </View>
  );
}

const styles = StyleSheet.create({
  loginContainer: {
    flex: 1,
    backgroundColor: '#0f172a', // Sleek dark aesthetic
    justifyContent: 'center',
    padding: 30,
  },
  headerBox: {
    alignItems: 'center',
    marginBottom: 50,
  },
  logoText: {
    fontSize: 42,
    fontWeight: '800',
    color: '#38bdf8',
    letterSpacing: 2,
  },
  subLogoText: {
    fontSize: 16,
    color: '#94a3b8',
    marginTop: 5,
    letterSpacing: 1,
  },
  formContainer: {
    backgroundColor: '#1e293b',
    padding: 25,
    borderRadius: 16,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 10 },
    shadowOpacity: 0.3,
    shadowRadius: 20,
    elevation: 10,
  },
  label: {
    color: '#cbd5e1',
    fontSize: 14,
    marginBottom: 8,
    fontWeight: '600',
  },
  input: {
    backgroundColor: '#0f172a',
    color: '#fff',
    borderRadius: 8,
    padding: 15,
    fontSize: 16,
    marginBottom: 20,
    borderWidth: 1,
    borderColor: '#334155',
  },
  loginButton: {
    backgroundColor: '#0284c7',
    padding: 16,
    borderRadius: 8,
    alignItems: 'center',
    marginTop: 10,
  },
  loginButtonText: {
    color: '#fff',
    fontWeight: '700',
    fontSize: 16,
    letterSpacing: 1,
  },
  footerText: {
    position: 'absolute',
    bottom: 40,
    alignSelf: 'center',
    color: '#475569',
    fontSize: 12,
  },
  // Dashboard Styles
  dashboardContainer: {
    flex: 1,
    backgroundColor: '#f8fafc',
    paddingTop: Platform.OS === 'ios' ? 50 : StatusBar.currentHeight,
  },
  dashHeader: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    padding: 15,
    backgroundColor: '#fff',
    borderBottomWidth: 1,
    borderBottomColor: '#e2e8f0',
  },
  dashTitle: {
    fontSize: 20,
    fontWeight: '700',
    color: '#1e293b',
  },
  logoutText: {
    color: '#ef4444',
    fontWeight: '600',
  },
  webview: {
    flex: 1,
    backgroundColor: 'transparent',
  },
  connectingOverlay: {
    position: 'absolute',
    top: 80, left: 0, right: 0, bottom: 0,
    backgroundColor: '#f8fafc',
    justifyContent: 'center',
    alignItems: 'center',
    zIndex: 10,
    padding: 30,
  },
  overlayTitle: {
    fontSize: 22,
    fontWeight: 'bold',
    marginTop: 20,
    color: '#334155',
  },
  overlaySub: {
    fontSize: 16,
    color: '#64748b',
    marginTop: 10,
    textAlign: 'center',
  },
  highlightText: {
    fontSize: 20,
    fontWeight: 'bold',
    color: '#0284c7',
    marginVertical: 10,
  },
});
