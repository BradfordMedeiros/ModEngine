
(define letterboxlength 0.5)
(define starttime 0)

(define letterbox-on #f)

(define (percent-elapsed) 
  (define amount (/ (- (time-seconds) starttime) letterboxlength))
  (if (> amount 1) 1 amount)
)

(define (toggle-elapsed)
  (define amount (percent-elapsed))
  (if letterbox-on amount (- 1 amount))
)
  
(define (amount) (* 0.1 (toggle-elapsed)))

(define (onFrame)
  (format #t "on key thing\n")
  (set-wstate (list 
    (list "$borders" "uvamount" (amount))
  ))

)

(define (toggle-letter-box)
  (set! letterbox-on (not letterbox-on))
  (set! starttime (time-seconds))
)
(define (onKey key scancode action mods)
  (if (and (= key 257) (= action 1))
    (toggle-letter-box)
  )
)

