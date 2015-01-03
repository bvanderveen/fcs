(defproject sim "0.0.2-SNAPSHOT"
  :description "A simple physics simulator"
  :url "http://github.com/bvanderveen/fcs"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :dependencies [
                 [org.clojure/clojure "1.5.1"]
                 [org.clojure/clojure-contrib "1.2.0"]
                 [org.clojure/tools.namespace "0.2.4"]
                 [quil "1.6.0"]]
  :main sim.core)
