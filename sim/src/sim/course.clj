(ns sim.course
  (:require
   [clojure.contrib.generic.math-functions :as math]))

(def course-initial-state
  {
   :latitude 0
   :longitude 0
   :velocity 0.0003
   :heading 0
   :roll 0
   })

(defn bound
  ([n low high] (min (max n low) high))
  ([n absolute] (bound n (* -1 absolute) absolute)))

(defn course-fn [state aileron elevator dt]
  {
   :latitude (+ (:latitude state) (* dt (math/cos (:heading state)) (:velocity state)))
   :longitude (+ (:longitude state) (* dt (math/sin (:heading state)) (:velocity state)))
   :velocity (:velocity state)
   :heading (+ (:heading state) (* dt (:roll state)))
   :roll (bound
          (+ (:roll state) (* dt aileron))
          (/ Math/PI 6)) ; don't bank past 30 degrees
   })
