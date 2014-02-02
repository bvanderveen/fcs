(ns monitor.io
  (:require
   [monitor.udp :as udp]
   [clojure.string :as string]
   [clojure.data.json :as json])
  (:import java.net.SocketTimeoutException))

(defn configuration-from-string [configuration-string]
  (let [parts (string/split configuration-string #":")]
    {
     :recv-addr "0.0.0.0"
     :recv-port (. Integer parseInt (nth parts 0 "49000"))
     :send-addr (nth parts 1 "0.0.0.0")
     :send-port (. Integer parseInt (nth parts 2 "49000"))
     }))

(defn from-string [string timeout]
  (let [settings (configuration-from-string string)
        send-tuple (udp/make-send (:send-addr settings) (:send-port settings) timeout)
        recv-tuple (udp/make-receive (:recv-port settings) timeout)]
    {
     :send (first send-tuple)
     :send-close (second send-tuple)
     :recv (first recv-tuple)
     :recv-close (second recv-tuple)
     }))

(defn close [io]
  (apply (:send-close io) [])
  (apply (:recv-close io) []))

(defn write-json [state]
  (json/write-str state))

(defn read-json [json-string]
  (if (nil? json-string)
    nil
    (json/read-str json-string :key-fn keyword)))

(defn do-read [io]
  (try (apply (:recv io) []) (catch SocketTimeoutException e nil)))

(defn read-input! [io]
  (let [data (do-read io)]
    (read-json data)))

(defn write-output! [io state]
  (try (apply (:send io) [(write-json state)]) (catch SocketTimeoutException e nil)))

