(defproject websockify "1.0.0-SNAPSHOT"
  :description "Clojure implementation of Websockify"
  :url "https://github.com/kanaka/websockify"
  :dependencies [[org.clojure/clojure "1.2.1"]
                 [org.clojure/tools.cli "0.2.1"]
                 [ring/ring-jetty-adapter "1.0.0-beta2"]
                 [org.eclipse.jetty/jetty-websocket "7.5.4.v20111024"]
                 [org.eclipse.jetty/jetty-server "7.5.4.v20111024"]
                 [org.eclipse.jetty/jetty-servlet "7.5.4.v20111024"]
                 [org.jboss.netty/netty "3.2.5.Final"]]
  ;:dev-dependencies [[swank-clojure "1.3.0-SNAPSHOT"]]
  :main websockify
  )
