import json
from datetime import datetime, timezone

import paho.mqtt.client as mqtt


BROKER = "localhost"
PORT = 1883
TOPIC = "garden/+/telemetry"


def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected to MQTT broker: {reason_code}")

    client.subscribe(TOPIC)

    print(f"Subscribed to: {TOPIC}")


def on_message(client, userdata, message):
    payload = message.payload.decode("utf-8")

    print("\nMQTT message received")
    print("Topic:", message.topic)
    print("Raw payload:", payload)

    try:
        data = json.loads(payload)

        node = data["node"]
        readings = data["readings"]

        received_at = datetime.now(timezone.utc)

        processed_data = {
            "node": node,
            "readings": readings,
            "received_at": received_at.isoformat()
        }

        print("Node:", node)
        print("Water temperature:", readings.get("water_temperature"))
        print("Received at:", received_at.isoformat())
        print("Processed data:", processed_data)

    except json.JSONDecodeError:
        print("ERROR: Message is not valid JSON")


client = mqtt.Client(
    mqtt.CallbackAPIVersion.VERSION2,
    client_id="garden_processor"
)

client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)

print("Waiting for sensor data...")

client.loop_forever()