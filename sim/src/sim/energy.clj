(ns sim.course
  (:require
   [clojure.contrib.generic.math-functions :as math]))

(def energy-initial-state
  {
   :x 0
   :y 0
   :xdot 0
   :ydot 0

   :alpha 0
   :theta 0
   :thetadot 0

   :epr 0
   })

(defn bound
  ([n low high] (min (max n low) high))
  ([n absolute] (bound n (* -1 absolute) absolute)))

(defn energy-fn [state elevator throttle dt]
  (let [
        xdot (:xdot state)
        ydot (:ydot state)
        speed-squard (+ (* xdot xdot) (* ydot ydot))
        drag]
    {
      :x (+ (:x state) (* dt (:xdot state)))
      :y (+ (:y state) (* dt (:ydot state)))
      :xdot nil
    }))
