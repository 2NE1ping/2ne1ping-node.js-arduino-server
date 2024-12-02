// express 불러오기
const express = require("express");
// express 인스턴스 생성
const app = express();
// 포트 정보
const httpPort = 3000;

// cors 불러오기
const cors = require("cors");

// CORS 미들웨어 사용
app.use(cors());

// serialport 및 websocket 불러오기
const { SerialPort } = require("serialport");
const WebSocket = require("ws");

const { exec } = require("child_process");

function uploadArduinoSketch(filePath) {
  const compileCommand =
    "arduino-cli compile --fqbn arduino:avr:uno " + filePath;
  const uploadCommand =
    "arduino-cli upload -p /dev/cu.usbmodem1401 --fqbn arduino:avr:uno " +
    filePath;

  exec(compileCommand, (error, stdout, stderr) => {
    if (error) {
      console.error(`Compile error: ${error.message}`);
      return;
    }
    console.log(`Compile output: ${stdout}`);

    exec(uploadCommand, (error, stdout, stderr) => {
      if (error) {
        console.error(`Upload error: ${error.message}`);
        return;
      }
      console.log(`Upload output: ${stdout}`);
    });
  });
}

// 아두이노 포트 찾기
async function findArduinoPort() {
  try {
    const ports = await SerialPort.list();
    const arduinoPort = ports.find(
      (port) => port.manufacturer && port.manufacturer.includes("Arduino")
    );
    return arduinoPort ? arduinoPort.path : null;
  } catch (error) {
    console.error("Error listing ports:", error);
    return null;
  }
}

// 웹소켓 서버 설정
(async () => {
  const arduinoPath = await findArduinoPort();
  if (!arduinoPath) {
    console.error("Arduino not found");
    return;
  }

  const serialPort = new SerialPort({ path: arduinoPath, baudRate: 9600 });
  const wss = new WebSocket.Server({ port: 8080 });

  let latestData = ""; // Buffer to store the latest data

  serialPort.on("data", (data) => {
    latestData = data.toString(); // Update the buffer with the latest data
  });

  wss.on("connection", (ws) => {
    console.log("Client connected");

    const interval = setInterval(() => {
      if (latestData) {
        console.log("Sending data:", latestData);
        ws.send(latestData);
      }
    }, 5000); // Send data every 5 seconds

    ws.on("close", () => {
      console.log("Client disconnected");
      clearInterval(interval); // Clear interval when client disconnects
    });
  });

  console.log("WebSocket server running on ws://localhost:8080");
})();

// 라우트 설정
app.get("/", (req, res) => {
  res.send("Hello World!");
});

app.post("/upload-sketch", (req, res) => {
  uploadArduinoSketch("arduino/hci_2ne1ping_start/hci_2ne1ping_start.ino");
  res.send("Arduino sketch upload initiated.");
});

app.post("/upload-sketch/:path", (req, res) => {
  const path = req.params.path;
  if (path === "camera") {
    uploadArduinoSketch("arduino/hci_2ne1ping_camera/hci_2ne1ping_camera.ino");
  } else if (path === "muse2") {
    uploadArduinoSketch("arduino/hci_2ne1ping_muse2/hci_2ne1ping_muse2.ino");
  } else {
    return res.status(404).send("Path not found");
  }
  res.send("Arduino sketch upload initiated.");
});

// 서버 실행
app.listen(httpPort, () => {
  console.log(`App running on port ${httpPort}...`);
});
