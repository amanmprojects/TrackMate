import requests
import time
import json

ESP32_IP = "192.168.131.81"  # Replace with your ESP32's IP address
URL = f"http://{ESP32_IP}/gps"

def get_gps_data():
    try:
        response = requests.get(URL, timeout=5)
        response.raise_for_status()
        data = response.json()
        return data["latitude"], data["longitude"], data["satellites"]
    except requests.exceptions.RequestException as e:
        print(f"Error: {e}")
    except Exception:
        print("Failed to retrieve GPS data")
        raise
        
    return None, None

import folium
from folium.plugins import MarkerCluster

def plot_cat_location(lat, lon, zoom=20):
    # Create a map centered on the cat's location
    cat_map = folium.Map(location=[lat, lon], zoom_start=zoom)
    
    # Add a marker for the cat's location
    folium.Marker(
        [lat, lon],
        popup="Cat's Location",
        icon=folium.Icon(color="red", icon="paw", prefix="fa")
    ).add_to(cat_map)
    
    # Save the map to an HTML file
    cat_map.save("cat_location.html")
    
    return cat_map

# # Example usage
# lat, lon = 19.295540, 72.854271  # Example coordinates
# plot_cat_location(lat, lon)


def main():
    while True:
        try:
            lat, lon, sats = get_gps_data()
        except Exception:
            lat, lon = None, None
            sats = 0
            time.sleep(2)
            continue
        if lat is not None and lon is not None:
            print(f"Latitude: {lat:.6f}, Longitude: {lon:.6f}, Satellites: {sats}")
            plot_cat_location(lat, lon)
            # print(type(plot_cat_location(lat, lon)))
        else:
            print("Failed to retrieve GPS data")
        time.sleep(2)

if __name__ == "__main__":
    main()
