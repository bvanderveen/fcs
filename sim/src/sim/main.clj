(ns sim.main
  (:require
   [sim.io :as io]
   [sim.core :as core]
   [sim.course :as course]))

(defn add-course-module [state]
  (core/add-module state course/course-initial-state course/course-fn))

(def state-atom (atom (add-course-module core/initial-state)))

(defn start! []
    (let [a (agent [] :error-handler (fn [a e] (println "agent error" e)))]
    (do
      (core/start! state-atom)
      (send-off a core/loop! state-atom #(io/from-string "5001:127.0.0.1:5000" 1000)))))

(start!)
(core/stop! state-atom)
