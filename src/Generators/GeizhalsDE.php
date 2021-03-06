<?php

namespace ElasticExport\Generators;

use Plenty\Modules\DataExchange\Contracts\CSVGenerator;
use Plenty\Modules\Helper\Services\ArrayHelper;
use Plenty\Modules\Helper\Models\KeyValue;
use Plenty\Modules\Item\DataLayer\Models\Record;
use Plenty\Modules\Item\DataLayer\Models\RecordList;
use Plenty\Modules\DataExchange\Models\FormatSetting;
use ElasticExport\Helper\ElasticExportHelper;
use Plenty\Modules\Order\Payment\Method\Models\PaymentMethod;

class GeizhalsDE extends CSVGenerator
{
	/*
     * @var ElasticExportHelper
     */
    private $elasticExportHelper;

	/*
	 * @var ArrayHelper
	 */
	private $arrayHelper;

	/**
     * Geizhals constructor.
     * @param ElasticExportHelper $elasticExportHelper
     * @param ArrayHelper $arrayHelper
     */
    public function __construct(ElasticExportHelper $elasticExportHelper, ArrayHelper $arrayHelper)
    {
        $this->elasticExportHelper = $elasticExportHelper;
		$this->arrayHelper = $arrayHelper;
    }

    /**
     * @param RecordList $resultData
     * @param array $formatSettings
     */
    protected function generateContent($resultData, array $formatSettings = [])
	{
		if($resultData instanceof RecordList)
		{
			$settings = $this->arrayHelper->buildMapFromObjectList($formatSettings, 'key', 'value');

			$this->setDelimiter(";");

			$this->addCSVContent([
				'Hersteller',
				'Produktcode',
				'Bezeichnung',
				'Preis',
				'Deeplink',
				'Vorkasse',
				'Nachnahme',
				'Verfügbarkeit',
				'Herstellercode',
				'EAN',
				'Kategorie',
				'Grundpreis',
			]);

			foreach($resultData as $item)
			{
                if(!$this->valid($item))
                {
                    continue;
                }

				$variationName = $this->elasticExportHelper->getAttributeValueSetShortFrontendName($item, $settings);
                $paymentInAdvance = $this->elasticExportHelper->getShippingCost($item, $settings, 0);
                $cashOnDelivery = $this->elasticExportHelper->getShippingCost($item, $settings, 1);

                if(!is_null($paymentInAdvance))
                {
                    $paymentInAdvance = number_format((float)$paymentInAdvance + $this->getPaymentShippingExtraCharge($item, $settings, 0), 2, '.', '');
                }
                else
                {
                    $paymentInAdvance = '';
                }

                if(!is_null($cashOnDelivery))
                {
                    $cashOnDelivery = number_format((float)$cashOnDelivery + $this->getPaymentShippingExtraCharge($item, $settings, 1), 2, '.', '');
                }
                else
                {
                    $cashOnDelivery = '';
                }

				$data = [
					'Hersteller' 		=> $this->elasticExportHelper->getExternalManufacturerName((int)$item->itemBase->producerId),
					'Produktcode' 		=> $item->itemBase->id,
					'Bezeichnung' 		=> $this->elasticExportHelper->getName($item, $settings) . (strlen($variationName) ? ' ' . $variationName : ''),
					'Preis' 			=> number_format((float)$this->elasticExportHelper->getPrice($item), 2, '.', ''),
					'Deeplink' 			=> $this->elasticExportHelper->getUrl($item, $settings, true, false),
					'Vorkasse' 			=> $paymentInAdvance,
					'Nachnahme' 		=> $cashOnDelivery,
					'Verfügbarkeit' 	=> $this->elasticExportHelper->getAvailability($item, $settings),
					'Herstellercode' 	=> $item->variationBase->model,
					'EAN' 				=> $this->elasticExportHelper->getBarcodeByType($item, $settings->get('barcode')),
					'Kategorie' 		=> $this->elasticExportHelper->getCategory((int)$item->variationStandardCategory->categoryId, $settings->get('lang'), $settings->get('plentyId')),
					'Grundpreis' 		=> $this->elasticExportHelper->getBasePrice($item, $settings),
				];

				$this->addCSVContent(array_values($data));
			}
		}
	}

	/**
	 * Check if item is valid.
	 * @param  Record $item
	 * @return bool
	 */
	private function valid(Record $item):bool
	{
		if($item->variationStock->stockNet <= 0 && $item->variationBase->limitOrderByStockSelect == 1)
		{
			return false;
		}

		return true;
	}

	/**
     * Get payement extra charge.
     * @param  Record   $item
     * @param  KeyValue $settings
     * @param  int      $paymentMethodId
     * @return float
     */
    private function getPaymentShippingExtraCharge(Record $item, KeyValue $settings, int $paymentMethodId):float
    {
        $paymentMethods = $this->elasticExportHelper->getPaymentMethods($settings);

        if(count($paymentMethods) > 0)
		{
            if(array_key_exists($paymentMethodId, $paymentMethods) && $paymentMethods[$paymentMethodId] instanceof PaymentMethod)
            {
                if($paymentMethods[$paymentMethodId]->feeForeignPercentageWebstore)
                {
                    return ((float) $paymentMethods[$paymentMethodId]->feeForeignPercentageWebstore / 100) * $this->elasticExportHelper->getPrice($item);
                }
                else
                {
                    return (float) $paymentMethods[$paymentMethodId]->feeForeignFlatRateWebstore;
                }
            }
		}

        return 0.0;
    }
}
