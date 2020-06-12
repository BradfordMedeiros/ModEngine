
(define (onTcpMessage message)
  (display (string-append "message is: " message "\n"))
)

(define (onKey key scancode action mods)
  (if (and (= key 257) (= action 1))
    (send-udp "test message 1")
  )
)


