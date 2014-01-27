(ns sim.core
  (:require
   [sim.udp :as udp]
   [clojure.string :as string]
   [clojure.contrib.generic.math-functions :as math]
   [clojure.data.json :as json])
  (:gen-class :main true))

(def initial-state {
                    :x 0
                    :y 0
                    :velocity 1
                    :heading 0
                    :heading-rate 0 })


(def sim-running (atom false))

(defn start-sim []
  (swap! sim-running (fn [_] true)))

(defn stop-sim []
  (swap! sim-running (fn [_] false)))


(start-sim)
(stop-sim)
sim-running

(def sim-state (atom initial-state))


(defn move [state turn dt]
  {
   :x (+ (:x state) (* dt (math/cos (:heading state)) (:velocity state)))
   :y (+ (:y state) (* dt (math/sin (:heading state)) (:velocity state)))
   :velocity (:velocity state)
   :heading (+ (:heading state) (* dt (:heading-rate state)))
   :heading-rate (+ turn (:heading-rate state))
   }
  )


(defn update-sim! [turn dt]
  (swap! sim-state (fn [s t d] (move s t d)) turn dt))


(update-sim! 0.000 1)

(defn parse-args [args]
  (let [parts (string/split args #":")]
    {
     :recv-addr "0.0.0.0"
     :recv-port (. Integer parseInt (nth parts 0 "49000"))
     :send-addr (nth parts 1 "0.0.0.0")
     :send-port (. Integer parseInt (nth parts 2 "49000"))
     }))

(parse-args "4000:0.0.0.1:300")

(string/split "asd:asdf" #":")

(json/write-str {"asdf" "foo"})

(defn read-input [input]
  (let [j (json/read-str input)]
    {:turn ("state.effector.aileron" j)}
    ))

(defn write-output [state]
  (json/write-str {
                         "state.sensor.longitude" (:x state)
                         "state.sensor.latitude" (:y state)
                         "state.sensor.heading" (:heading state)
                         "state.senson.roll" (:heading-rate state)
                         }))

(defn simple [args]
  (let [
        settings (parse-args args)
        send (udp/make-send (:send-addr settings) (:send-port settings))
        recv (udp/make-receive (:recv-port settings))]
    (println "starting sim")
    (while @sim-running
      (let [in (read-input (recv))]
        (update-sim! (:turn in) 1)
        (write-output @sim-state)
        (println ".")
        ))
    (println "stopped.")))

(simple "4000:127.0.0.1:4000")

(defn -main [& args]
  (let [send (udp/make-send nil)]
    nil))
