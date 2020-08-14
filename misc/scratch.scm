
(define-syntax for 
  (syntax-rules (in)
     ((for x in y body ...) (for-each (lambda (fn) body ...) y))
  )
)


(for 1 in '(1 2 3)
  (display "yay\n")
  (display "another\n")
)
