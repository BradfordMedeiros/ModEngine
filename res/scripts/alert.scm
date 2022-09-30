(define (amount-to-draw text createTime rate)
  (define currIndex (inexact->exact (floor (* rate (- (time-seconds #t) createTime)))))
  (substring text 0 (min (string-length text) currIndex))
)

(define maxBufferSize 1)
(define messageBuffer (list))

(define displayMessage (args "alert"))
(define (onMessage key value)
  (if (equal? key "alert")
    (begin
      (set! messageBuffer (reverse (cons (list value (time-seconds #t)) (reverse messageBuffer)))) 
      (if (> (length messageBuffer) maxBufferSize)
        (set! messageBuffer (cdr messageBuffer))
      )
    )
  )
)

(define (createTimeForMessage message) (cadr message))

(define letterSize 8)
(define letterSizeNdi (/ letterSize 1000))
(define margin (* letterSizeNdi 3))
(define marginLeft margin)
(define marginBottom margin)
(define (render-alerts yoffset buffer)
  (if (> (length buffer) 0)
    (let (
        (text (car (car buffer)))
        (createTime (createTimeForMessage (car buffer)))
      )
      (begin
        (draw-text-ndi 
          (amount-to-draw text  createTime 100) 
          (+ -1 marginLeft) 
          (+ -1 (* letterSizeNdi 0.5) marginBottom) 
          letterSize
        )
        (render-alerts (+ yoffset 10) (cdr buffer))
      )
    )
  )
)

(define bufferExpirationTimeMs 5000)
(define (isNotExpiredMessage message) 
  (define currTime (time-seconds #t))
  (define createTime (createTimeForMessage message))
  (define diff (* 1000 (- currTime createTime)))
  (< diff bufferExpirationTimeMs)
)
(define (filterExpiredMessages) 
  (set! messageBuffer (filter isNotExpiredMessage messageBuffer))
)

(define (onFrame)
  (render-alerts 400 messageBuffer)
  (filterExpiredMessages) ; something like this should probably doesn't need to be done every frame (higher fps means more filtering)
)
