
(define (onFrame)
  (format #t "time: ~a\n" (time-seconds))
  (format #t "time realtime: ~a\n" (time-seconds #t))
  (format #t "elapsed: ~a\n" (time-elapsed))
  (format #t "\n")
)