(ns sim.udp
  (:import (java.net DatagramPacket DatagramSocket InetAddress)))

(defn set-timeout [socket timeout]
  (.setSoTimeout socket timeout)
  socket)

(defn make-socket
  ([] (new DatagramSocket))
  ([port] (new DatagramSocket port)))

(defn close-socket [s]
  (.close s))

(defn send-data [send-socket ip port data]
  (let [ipaddress (InetAddress/getByName ip)
        send-packet (new DatagramPacket (.getBytes data) (.length data) ipaddress port)]
    (.send send-socket send-packet)))

(defn receive-data [receive-socket]
  (let [receive-data (byte-array 1024)
        receive-packet (new DatagramPacket receive-data 1024)]
    (.receive receive-socket receive-packet)
    (new String (.getData receive-packet) 0 (.getLength receive-packet))))

(defn make-receive [receive-port timeout]
  (let [s (set-timeout (make-socket receive-port) timeout)]
    [(fn [] (receive-data s)) (fn [] (close-socket s))]))

(defn make-send [ip port timeout]
  (let [s (set-timeout (make-socket) timeout)]
    [(fn [data] (send-data s ip port data)) (fn [] (close-socket s))]))
