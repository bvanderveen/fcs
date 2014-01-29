(ns sim.main
  (:require
   [sim.io :as io]
   [sim.core :as core]
   [sim.course :as course]))

(comment
  (use '[clojure.tools.namespace.repl :only (refresh)])
  (refresh)
)

(defn add-course-module [state]
  (core/add-module state course/course-initial-state course/course-fn))

(def state-atom (atom (add-course-module core/initial-state)))

(defn start! []
  (let [io (io/from-string "4001:127.0.0.1:4000" 200)]
    (try
      (core/start! state-atom)
      (core/loop! state-atom io)
    (finally (io/close io)))))


(start!)
(comment )
(core/stop! state-atom)
(comment )



(defn -main [& args]
  (println "butts"))
