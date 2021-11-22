

(define currIndex 0)
(define exposureAmount (list 0.1 1 5))

(define (next-exposure)
  (define indexPlusOne (+ currIndex 1))
  (if (>= indexPlusOne (length exposureAmount))
    (set! indexPlusOne 0)
  )
  (set! currIndex indexPlusOne)
  (list-ref exposureAmount currIndex)
)
(define (onKeyChar codepoint) 
  (define exposureAmount (next-exposure))
  (if (equal? codepoint 101)
    (set-wstate (list 
      (list "exposure" "amount" exposureAmount)
    ))
  )
)
