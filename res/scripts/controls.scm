(define (onKey key scancode action mods)
  (display (string-append "Key is: " (number->string key) " (" (string (integer->char key)) ")" "\n"))
)