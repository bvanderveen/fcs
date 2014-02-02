(ns sim.course
  (:require
   [clojure.contrib.generic.math-functions :as math]))

(def course-initial-state
  {
   :latitude 0
   :longitude 0
   :velocity 0.0003
   :heading 0
   :heading-rate 0 })

(defn course-fn [state turn dt]
  {
   :latitude (+ (:latitude state) (* dt (math/cos (:heading state)) (:velocity state)))
   :longitude (+ (:longitude state) (* dt (math/sin (:heading state)) (:velocity state)))
   :velocity (:velocity state)
   :heading (+ (:heading state) (* dt (:heading-rate state)))
   :heading-rate (+ 0.000003 (if (nil? turn) 0 turn) (:heading-rate state))
   })
