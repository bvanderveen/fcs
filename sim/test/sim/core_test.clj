(ns sim.core-test
  (:use clojure.test
        sim.core))

(deftest test-parse-args
  (testing "port only sets recv port"
    (is (parse-args "4000")
           { :recv-port 4000
             :recv-addr "0.0.0.0"
             :send-port 49001
             :send-addr "0.0.0.0"}))
  (testing "port then addr sets recv port then send addr"
    (is (parse-args "4000:0.0.0.1")
           { :recv-port 4000
             :recv-addr "0.0.0.0"
             :send-port 49001
             :send-addr "0.0.0.1"}))
  (testing "port then addr sets recv port then send addr then send port"
    (is (parse-args "4000:0.0.0.1:5000")
           { :recv-port 4000
             :recv-addr "0.0.0.0"
             :send-port 5000
             :send-addr "0.0.0.1"})))

(run-tests)
