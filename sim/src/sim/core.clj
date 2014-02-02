(ns sim.core
  (:require
   [sim.io :as io]
   [clojure.string :as string]
   [clojure.contrib.generic.math-functions :as math]
   [clojure.data.json :as json]))

(def initial-state {
                    :running false
                    :modules []
                    :t 0
                    })

(defn add-module [state initial-state state-fn]
  (assoc state :modules (conj (:modules state) [initial-state state-fn])))

(defn get-dt [state]
  (- (System/currentTimeMillis) (:t state)))

(defn update-module [module input dt]
  (let [state (first module)
        state-fn (second module)]
    [(state-fn state input dt) state-fn]))

(defn update-fn [state input]
  (let [dt (get-dt state)]
    (assoc state
      :modules (map #(update-module % input dt) (:modules state))
      :t (+ (:t state) dt))))

(defn serializable-state [state]
  (reduce conj (map #(first %) (:modules state))))

(defn running? [state]
  (:running state))

(defn update! [state-atom input]
  (swap! state-atom input))

(defn loop! [a state-atom io-fn]
  (let [io (io-fn)]
    (try
      (do
        (println "sim started")
        (while (running? @state-atom)
          (let [input (io/read-input! io)]
            (swap! state-atom update-fn input)
            (let [output (serializable-state @state-atom)]
              (io/write-output! io output))
            ))
        (println "sim stopped"))
      (catch
        Exception e (println "sim got exception" e))
      (finally
       (io/close io)))))

(defn start! [state-atom]
  (swap! state-atom (fn [s] (assoc s :running true :t (System/currentTimeMillis)))))

(defn stop! [state-atom]
  (swap! state-atom (fn [s]
                      (println "stop!!!!")
                      (assoc s :running false))))
