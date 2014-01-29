(ns sim.course
  (:require
   [clojure.contrib.generic.math-functions :as math]))

(def course-initial-state
  {
   :x 0
   :y 0
   :velocity 1
   :heading 0
   :heading-rate 0 })

(defn course-fn [state turn dt]
  (println "course-fn" state turn dt)
  {
   :x (+ (:x state) (* dt (math/cos (:heading state)) (:velocity state)))
   :y (+ (:y state) (* dt (math/sin (:heading state)) (:velocity state)))
   :velocity (:velocity state)
   :heading (+ (:heading state) (* dt (:heading-rate state)))
   :heading-rate (+ turn (:heading-rate state))
   })
