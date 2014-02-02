(ns monitor.core
  (:require
   [monitor.io :as io])
  (:use quil.core))

(def flight-state-atom (atom {
                              :running false
                              :latitude 0
                              :longitude 0
                              :heading 0 }))

(defn flight-bounds [flight-state]
  [-100 100 -100 100])

(defn position-to-window [[x y] [view-min-x view-max-x view-min-y view-max-y]]
  (let [width (- view-max-x view-min-x)
        height (- view-max-y view-min-y)
        translated-x (- x view-min-x)
        translated-y (- y view-min-y)]
    [
     (/ translated-x width)
     (/ translated-y height)
     ]))


(defn draw-vehicle [flight-state]
  (let [lat-lon [(:latitude flight-state) (:longitude flight-state)]
        viewport (flight-bounds flight-state)
        position-in-window (position-to-window lat-lon viewport)]
    (push-matrix)
    (translate position-in-window)
    (scale 0.1 0.1)
    (rotate (:heading flight-state))
    (fill (color 0 0 255))
    (triangle 0.5 0.5 0 -0.5 -0.5 0.5)
    (pop-matrix)))

(defn draw-all [flight-state]
  (draw-vehicle flight-state))

(defn setup []
  (smooth)
  (frame-rate 1)
  (background 255))

(defn draw []
  (no-stroke)
  (fill 123)
  (scale (width) (height))
  (rect 0 0 1 1)
  (comment (swap! flight-state-atom (fn [s] (assoc s
                                              :latitude (+ 0.2 (:latitude s))
                                              :heading (+ 0.01 (:heading s))))))
  (draw-all @flight-state-atom)
  )

(defn comm-running? [state]
  (:running state))

(defn comm-stop! [state-atom]
  (swap! state-atom (fn [s] (assoc s :running false))))

(defn comm-start! [state-atom]
  (swap! state-atom (fn [s] (assoc s :running true))))

(defn comm-update-fn [state input]
  (let [s (identity state)
        source (if (nil? input) state input)]
    (assoc state
      :latitude (:latitude source)
      :longitude (:longitude source)
      :heading (:heading source))))

(defn comm-loop! [a state-atom io-fn]
  (let [io (io-fn)]
    (try
      (do
        (println "monitor comm started")
        (while (comm-running? @state-atom)
          (let [input (io/read-input! io)]
            (swap! state-atom comm-update-fn input)))
        (println "monitor comm stopped"))
      (catch
        Exception e (println "got exception" e))
      (finally
       (io/close io)))))

(defn comm-listen! []
  (let [a (agent [] :error-handler (fn [a e] (println "agent error" e)))]
    (do
      (comm-start! flight-state-atom)
      (send-off a comm-loop! flight-state-atom #(io/from-string "5000:127.0.0.1:5001" 1000)))))


(defn start-sketch! []
  (sketch
   :title "fcs monitor"
   :setup setup
   :draw draw
   :size [500 500]))


(comm-listen!)
(comm-stop! flight-state-atom)
(start-sketch!)
