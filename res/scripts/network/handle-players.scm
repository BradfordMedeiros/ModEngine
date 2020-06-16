


(define (on-player-join connection-hash)
  (display "scheme script -- player joined\n")
  (display "connection: ")
  (display connection-hash)
  (display "\n")
)

(define (on-player-leave connection-hash)
  (display "scheme script -- player leaved\n")
)